"""Managed worktree and governed task-packet operations."""

from __future__ import annotations

import hashlib
import json
import re
import shutil
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


class WorkflowError(RuntimeError):
    """Raised when a governed workflow invariant is violated."""


_TASK_ID_PATTERN = re.compile(r"^[A-Za-z0-9][A-Za-z0-9._-]{0,63}$")


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat()


def run_git(
    repository: Path,
    *arguments: str,
    check: bool = True,
) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", "-C", str(repository), *arguments],
        check=check,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


def repository_root(path: Path) -> Path:
    result = run_git(path, "rev-parse", "--show-toplevel")
    return Path(result.stdout.strip()).resolve()


def git_common_dir(path: Path) -> Path:
    result = run_git(path, "rev-parse", "--git-common-dir")
    value = Path(result.stdout.strip())
    if not value.is_absolute():
        value = repository_root(path) / value
    return value.resolve()


def is_primary_checkout(path: Path) -> bool:
    root = repository_root(path)
    git_dir_result = run_git(path, "rev-parse", "--git-dir")
    git_dir = Path(git_dir_result.stdout.strip())

    if not git_dir.is_absolute():
        git_dir = root / git_dir

    return git_dir.resolve() == git_common_dir(path)


def validate_task_id(task_id: str) -> str:
    if not _TASK_ID_PATTERN.fullmatch(task_id):
        raise WorkflowError(
            "Task ID must contain only letters, digits, '.', '_' or '-', "
            "must begin with a letter or digit, and must not exceed 64 characters."
        )
    return task_id


def canonical_json(payload: dict[str, Any]) -> bytes:
    return json.dumps(
        payload,
        sort_keys=True,
        separators=(",", ":"),
        ensure_ascii=False,
    ).encode("utf-8")


def sha256_payload(payload: dict[str, Any]) -> str:
    return hashlib.sha256(canonical_json(payload)).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def write_json(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        json.dumps(payload, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    temporary.replace(path)


def read_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as error:
        raise WorkflowError(f"Required file does not exist: {path}") from error
    except json.JSONDecodeError as error:
        raise WorkflowError(f"Invalid JSON file: {path}: {error}") from error

    if not isinstance(value, dict):
        raise WorkflowError(f"Expected a JSON object: {path}")

    return value


class ManagedWorkflow:
    """Creates and governs isolated task worktrees."""

    def __init__(self, repository: Path):
        self.repository = repository_root(repository)

        if not is_primary_checkout(self.repository):
            raise WorkflowError(
                "Workflow control must be invoked from the primary checkout."
            )

        self.control_root = self.repository / ".sovereign-mesh"
        self.worktrees_root = self.control_root / "worktrees"
        self.tasks_root = self.control_root / "tasks"
        self.evidence_root = self.control_root / "evidence"
        self.proposals_root = self.control_root / "proposals"
        self.decisions_root = self.control_root / "decisions"

    def task_path(self, task_id: str) -> Path:
        return self.tasks_root / f"{validate_task_id(task_id)}.json"

    def evidence_path(self, task_id: str) -> Path:
        return self.evidence_root / f"{validate_task_id(task_id)}.json"

    def proposal_path(self, task_id: str) -> Path:
        return self.proposals_root / f"{validate_task_id(task_id)}.json"

    def decision_path(self, task_id: str) -> Path:
        return self.decisions_root / f"{validate_task_id(task_id)}.decision.json"

    def worktree_path(self, task_id: str) -> Path:
        return self.worktrees_root / validate_task_id(task_id)

    def branch_name(self, task_id: str) -> str:
        return f"sovereign-mesh/{validate_task_id(task_id)}"

    def create_task(
        self,
        task_id: str,
        title: str,
        prompt: str,
        adapter: str,
        base_ref: str = "HEAD",
    ) -> dict[str, Any]:
        task_id = validate_task_id(task_id)
        task_file = self.task_path(task_id)

        if task_file.exists():
            raise WorkflowError(f"Task already exists: {task_id}")

        worktree = self.worktree_path(task_id)
        branch = self.branch_name(task_id)

        if worktree.exists():
            raise WorkflowError(f"Worktree path already exists: {worktree}")

        branch_check = run_git(
            self.repository,
            "show-ref",
            "--verify",
            "--quiet",
            f"refs/heads/{branch}",
            check=False,
        )
        if branch_check.returncode == 0:
            raise WorkflowError(f"Task branch already exists: {branch}")

        base_commit = run_git(
            self.repository,
            "rev-parse",
            "--verify",
            f"{base_ref}^{{commit}}",
        ).stdout.strip()

        worktree.parent.mkdir(parents=True, exist_ok=True)

        result = run_git(
            self.repository,
            "worktree",
            "add",
            "-b",
            branch,
            str(worktree),
            base_commit,
            check=False,
        )
        if result.returncode != 0:
            raise WorkflowError(
                "Unable to create managed worktree:\n"
                f"{result.stderr.strip() or result.stdout.strip()}"
            )

        task = {
            "schema_version": 1,
            "task_id": task_id,
            "title": title,
            "prompt": prompt,
            "adapter": adapter,
            "state": "prepared",
            "created_at": utc_now(),
            "base_commit": base_commit,
            "branch": branch,
            "worktree": str(worktree.resolve()),
            "human_principal": "sovereign-owner",
            "mutation_scope": "isolated-worktree-only",
            "integration_requires_approval": True,
        }
        task["task_digest"] = sha256_payload(task)

        write_json(task_file, task)
        return task

    def inspect_task(self, task_id: str) -> dict[str, Any]:
        task = read_json(self.task_path(task_id))
        worktree = Path(str(task["worktree"]))

        expected_digest = task.pop("task_digest", None)
        actual_digest = sha256_payload(task)
        task["task_digest"] = expected_digest

        worktree_exists = worktree.is_dir()
        branch = str(task["branch"])

        branch_check = run_git(
            self.repository,
            "show-ref",
            "--verify",
            "--quiet",
            f"refs/heads/{branch}",
            check=False,
        )

        status = ""
        head = None
        if worktree_exists:
            status = run_git(worktree, "status", "--short").stdout
            head = run_git(worktree, "rev-parse", "HEAD").stdout.strip()

        return {
            "task": task,
            "integrity": {
                "task_digest_valid": expected_digest == actual_digest,
                "expected_digest": expected_digest,
                "actual_digest": actual_digest,
            },
            "worktree": {
                "exists": worktree_exists,
                "is_primary_checkout": (
                    is_primary_checkout(worktree) if worktree_exists else None
                ),
                "branch_exists": branch_check.returncode == 0,
                "head": head,
                "status": status.splitlines(),
            },
        }

    def record_evidence(
        self,
        task_id: str,
        source_file: Path,
        producer: str,
        command_preview: str,
        source_identity: str | None = None,
    ) -> dict[str, Any]:
        task = read_json(self.task_path(task_id))
        inspection = self.inspect_task(task_id)

        if not inspection["integrity"]["task_digest_valid"]:
            raise WorkflowError("Task manifest integrity verification failed.")

        if not source_file.is_file():
            raise WorkflowError(f"Evidence source does not exist: {source_file}")

        evidence = {
            "schema_version": 1,
            "task_id": task_id,
            "task_digest": task["task_digest"],
            "producer": producer,
            "captured_at": utc_now(),
            "command_preview": command_preview,
            "source_file": str(source_file.resolve()),
            "source_sha256": sha256_file(source_file),
            "source_identity": source_identity,
            "content": source_file.read_text(encoding="utf-8"),
            "worktree_head": inspection["worktree"]["head"],
            "worktree_status": inspection["worktree"]["status"],
        }
        evidence["evidence_digest"] = sha256_payload(evidence)

        write_json(self.evidence_path(task_id), evidence)
        return evidence

    def assemble_proposal(
        self,
        task_id: str,
        summary: str,
        recommendation: str,
    ) -> dict[str, Any]:
        task = read_json(self.task_path(task_id))
        evidence = read_json(self.evidence_path(task_id))
        inspection = self.inspect_task(task_id)

        if not inspection["integrity"]["task_digest_valid"]:
            raise WorkflowError("Task manifest integrity verification failed.")

        evidence_digest = evidence.get("evidence_digest")
        evidence_body = dict(evidence)
        evidence_body.pop("evidence_digest", None)

        if evidence_digest != sha256_payload(evidence_body):
            raise WorkflowError("Evidence integrity verification failed.")

        if evidence.get("task_digest") != task.get("task_digest"):
            raise WorkflowError("Evidence is not bound to the current task manifest.")

        execution = None
        source_identity = evidence.get("source_identity")
        if source_identity is not None:
            try:
                execution_record = json.loads(evidence["content"])
            except (KeyError, json.JSONDecodeError) as error:
                raise WorkflowError(
                    "Governed execution evidence contains invalid JSON."
                ) from error

            if not isinstance(execution_record, dict):
                raise WorkflowError(
                    "Governed execution evidence must contain a JSON object."
                )

            execution_digest = execution_record.get("execution_digest")
            execution_body = dict(execution_record)
            execution_body.pop("execution_digest", None)

            if execution_digest != sha256_payload(execution_body):
                raise WorkflowError(
                    "Governed execution record integrity verification failed."
                )

            if execution_digest != source_identity:
                raise WorkflowError(
                    "Evidence source identity is not bound to its execution record."
                )

            if execution_record.get("task_id") != task_id:
                raise WorkflowError(
                    "Governed execution record is bound to another task."
                )

            if execution_record.get("task_digest") != task.get("task_digest"):
                raise WorkflowError(
                    "Governed execution record is not bound to the current task."
                )

            execution = {
                "execution_id": execution_record.get("execution_id"),
                "execution_digest": execution_digest,
                "command_display": execution_record.get("command_display"),
                "returncode": execution_record.get("returncode"),
                "succeeded": execution_record.get("succeeded"),
                "timed_out": execution_record.get("timed_out"),
                "worktree_head": execution_record.get("worktree_head"),
            }

        proposal = {
            "schema_version": 1,
            "proposal_id": task_id,
            "task_id": task_id,
            "created_at": utc_now(),
            "base_commit": task["base_commit"],
            "worktree_head": inspection["worktree"]["head"],
            "branch": task["branch"],
            "summary": summary,
            "recommendation": recommendation,
            "task_digest": task["task_digest"],
            "evidence_digest": evidence_digest,
            "execution": execution,
            "integration_state": "awaiting-human-decision",
            "human_principal": task["human_principal"],
        }
        proposal["proposal_digest"] = sha256_payload(proposal)

        write_json(self.proposal_path(task_id), proposal)
        return proposal

    def verify_integration_authority(self, task_id: str) -> dict[str, Any]:
        proposal = read_json(self.proposal_path(task_id))
        decision = read_json(self.decision_path(task_id))

        proposal_digest = proposal.get("proposal_digest")
        proposal_body = dict(proposal)
        proposal_body.pop("proposal_digest", None)

        proposal_valid = proposal_digest == sha256_payload(proposal_body)
        approved = decision.get("decision") == "approved"
        bound = decision.get("proposal_digest") == proposal_digest
        principal_valid = (
            decision.get("principal") == proposal.get("human_principal")
        )

        authorized = proposal_valid and approved and bound and principal_valid

        return {
            "authorized": authorized,
            "proposal_digest_valid": proposal_valid,
            "decision_is_approved": approved,
            "decision_bound_to_proposal": bound,
            "principal_valid": principal_valid,
            "integration_command_preview": (
                f"git -C {self.repository} merge --no-ff "
                f"{proposal['branch']}"
                if authorized
                else None
            ),
        }

    def remove_task_worktree(
        self,
        task_id: str,
        delete_branch: bool = False,
    ) -> dict[str, Any]:
        task = read_json(self.task_path(task_id))
        worktree = Path(str(task["worktree"]))
        branch = str(task["branch"])

        if worktree.exists():
            status = run_git(worktree, "status", "--porcelain").stdout.strip()
            if status:
                raise WorkflowError(
                    "Refusing to remove a worktree with uncommitted changes."
                )

            result = run_git(
                self.repository,
                "worktree",
                "remove",
                str(worktree),
                check=False,
            )
            if result.returncode != 0:
                raise WorkflowError(
                    result.stderr.strip()
                    or "Unable to remove managed worktree."
                )

        branch_deleted = False
        if delete_branch:
            result = run_git(
                self.repository,
                "branch",
                "-D",
                branch,
                check=False,
            )
            branch_deleted = result.returncode == 0

        if worktree.exists():
            shutil.rmtree(worktree)

        return {
            "task_id": task_id,
            "worktree_removed": not worktree.exists(),
            "branch_deleted": branch_deleted,
        }
