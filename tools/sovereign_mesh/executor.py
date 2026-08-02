"""Governed command execution for managed Sovereign Mesh worktrees."""

from __future__ import annotations

import hashlib
import json
import os
import shlex
import shutil
import subprocess
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Sequence

from .policy import BuildAuthorityPolicy
from .repository import require_isolated_worktree
from .workflow import ManagedWorkflow, WorkflowError


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat()


def canonical_json(payload: dict[str, Any]) -> str:
    return json.dumps(
        payload,
        sort_keys=True,
        separators=(",", ":"),
        ensure_ascii=False,
    )


def sha256_payload(payload: dict[str, Any]) -> str:
    return hashlib.sha256(
        canonical_json(payload).encode("utf-8")
    ).hexdigest()


def write_json_atomic(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    os.replace(temporary, path)


class GovernedExecutor:
    """Execute explicit argv inside a validated managed worktree."""

    def __init__(self, workflow: ManagedWorkflow) -> None:
        self.workflow = workflow
        policy_path = (
            Path(__file__).resolve().parent
            / "policy"
            / "build-authority.json"
        )
        self.policy = BuildAuthorityPolicy.load(policy_path)

    @staticmethod
    def filtered_environment() -> dict[str, str]:
        allowed = {
            "HOME",
            "LANG",
            "LC_ALL",
            "PATH",
            "PYTHONPATH",
            "TERM",
            "TMPDIR",
            "USER",
        }
        environment = {
            key: value
            for key, value in os.environ.items()
            if key in allowed
        }
        environment["GIT_TERMINAL_PROMPT"] = "0"
        environment["SOVEREIGN_MESH_EXECUTION"] = "governed"
        return environment

    @staticmethod
    def run_git(worktree: Path, *arguments: str) -> str:
        result = subprocess.run(
            ["git", "-C", str(worktree), *arguments],
            check=False,
            capture_output=True,
            text=True,
            timeout=30,
        )
        return result.stdout

    def execute(
        self,
        *,
        task_id: str,
        argv: Sequence[str],
        timeout_seconds: int = 900,
        cwd_relative: str = ".",
    ) -> dict[str, Any]:
        if not argv:
            raise WorkflowError("Execution argv must not be empty.")

        if timeout_seconds < 1 or timeout_seconds > 86400:
            raise WorkflowError(
                "Timeout must be between 1 and 86400 seconds."
            )

        inspection = self.workflow.inspect_task(task_id)

        if not inspection["integrity"]["task_digest_valid"]:
            raise WorkflowError(
                "Task manifest integrity verification failed."
            )

        if not inspection["worktree"]["exists"]:
            raise WorkflowError(
                "Managed task worktree does not exist."
            )

        task = inspection["task"]
        worktree = Path(str(task["worktree"])).resolve()
        require_isolated_worktree(worktree)

        cwd = (worktree / cwd_relative).resolve()
        try:
            cwd.relative_to(worktree)
        except ValueError as error:
            raise WorkflowError(
                "Execution cwd escapes the managed worktree."
            ) from error

        if not cwd.is_dir():
            raise WorkflowError(
                f"Execution cwd does not exist: {cwd}"
            )

        normalized_argv = tuple(str(value) for value in argv)
        self.policy.validate_argv(normalized_argv)

        environment = self.filtered_environment()
        executable = shutil.which(
            normalized_argv[0],
            path=environment.get("PATH"),
        )
        if executable is None:
            raise WorkflowError(
                "Execution executable is unavailable: "
                f"{normalized_argv[0]}"
            )

        command = (executable, *normalized_argv[1:])
        execution_id = datetime.now(timezone.utc).strftime(
            "%Y%m%dT%H%M%S.%fZ"
        )

        execution_directory = (
            self.workflow.task_path(task_id).parent
            / "executions"
            / task_id
        )
        record_path = execution_directory / f"{execution_id}.json"
        stdout_path = execution_directory / (
            f"{execution_id}.stdout.log"
        )
        stderr_path = execution_directory / (
            f"{execution_id}.stderr.log"
        )

        started_at = utc_now()
        started_monotonic = time.monotonic()
        timed_out = False

        try:
            completed = subprocess.run(
                command,
                cwd=cwd,
                env=environment,
                check=False,
                capture_output=True,
                text=True,
                timeout=timeout_seconds,
                shell=False,
            )
            returncode = completed.returncode
            stdout = completed.stdout
            stderr = completed.stderr
        except subprocess.TimeoutExpired as error:
            timed_out = True
            returncode = 124
            stdout = error.stdout or ""
            stderr = error.stderr or ""

            if isinstance(stdout, bytes):
                stdout = stdout.decode(
                    "utf-8",
                    errors="replace",
                )
            if isinstance(stderr, bytes):
                stderr = stderr.decode(
                    "utf-8",
                    errors="replace",
                )

        finished_at = utc_now()
        duration_seconds = round(
            time.monotonic() - started_monotonic,
            6,
        )

        stdout_path.parent.mkdir(
            parents=True,
            exist_ok=True,
        )
        stdout_path.write_text(stdout, encoding="utf-8")
        stderr_path.write_text(stderr, encoding="utf-8")

        record: dict[str, Any] = {
            "schema_version": 1,
            "execution_id": execution_id,
            "task_id": task_id,
            "task_digest": task["task_digest"],
            "adapter": task["adapter"],
            "argv": list(command),
            "command_display": shlex.join(command),
            "cwd": str(cwd),
            "worktree": str(worktree),
            "worktree_head": self.run_git(
                worktree,
                "rev-parse",
                "HEAD",
            ).strip(),
            "worktree_status": self.run_git(
                worktree,
                "status",
                "--short",
            ).splitlines(),
            "git_diff": self.run_git(
                worktree,
                "diff",
                "--binary",
            ),
            "started_at": started_at,
            "finished_at": finished_at,
            "duration_seconds": duration_seconds,
            "timeout_seconds": timeout_seconds,
            "timed_out": timed_out,
            "returncode": returncode,
            "succeeded": returncode == 0 and not timed_out,
            "stdout_path": str(stdout_path.resolve()),
            "stderr_path": str(stderr_path.resolve()),
            "stdout_sha256": hashlib.sha256(
                stdout.encode("utf-8")
            ).hexdigest(),
            "stderr_sha256": hashlib.sha256(
                stderr.encode("utf-8")
            ).hexdigest(),
        }
        record["execution_digest"] = sha256_payload(record)

        write_json_atomic(record_path, record)

        evidence = self.workflow.record_evidence(
            task_id=task_id,
            source_file=record_path,
            producer=f"{task['adapter']}:governed-executor",
            command_preview=record["command_display"],
            source_identity=record["execution_digest"],
        )

        return {
            "record": record,
            "record_path": str(record_path.resolve()),
            "evidence": evidence,
            "evidence_path": str(
                self.workflow.evidence_path(task_id).resolve()
            ),
        }
