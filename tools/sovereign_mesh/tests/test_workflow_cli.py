from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


def git(path: Path, *arguments: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(path), *arguments],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return result.stdout.strip()


class WorkflowCliExecutionTest(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.repository = Path(self.temporary.name) / "repository"
        self.repository.mkdir()

        git(self.repository, "init")
        git(self.repository, "config", "user.name", "Sovereign Test")
        git(
            self.repository,
            "config",
            "user.email",
            "sovereign@example.invalid",
        )

        (self.repository / "README.md").write_text(
            "Sovereign test repository\n",
            encoding="utf-8",
        )
        git(self.repository, "add", "README.md")
        git(self.repository, "commit", "-m", "initial")

        self.project_root = Path(__file__).resolve().parents[3]
        self.workflowctl = (
            self.project_root / "tools" / "sovereign_mesh" / "workflowctl"
        )

        environment = dict(os.environ)
        tools_path = str(self.project_root / "tools")
        current = environment.get("PYTHONPATH")
        environment["PYTHONPATH"] = (
            tools_path if not current else f"{tools_path}:{current}"
        )
        self.environment = environment

        created = self.run_cli(
            "create",
            "--task-id",
            "SM-CLI",
            "--title",
            "CLI execution test",
            "--prompt",
            "Execute a governed command.",
            "--adapter",
            "codex",
        )
        self.assertEqual(created.returncode, 0, created.stderr)

    def tearDown(self) -> None:
        subprocess.run(
            ["git", "-C", str(self.repository), "worktree", "prune"],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        self.temporary.cleanup()

    def run_cli(self, *arguments: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(
            [
                str(self.workflowctl),
                "--repository",
                str(self.repository),
                *arguments,
            ],
            check=False,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=self.environment,
        )

    def test_execute_success_returns_zero_and_json(self) -> None:
        result = self.run_cli(
            "execute",
            "--task-id",
            "SM-CLI",
            "--",
            sys.executable,
            "-c",
            "print('cli success')",
        )

        self.assertEqual(result.returncode, 0, result.stderr)
        payload = json.loads(result.stdout)
        self.assertTrue(payload["record"]["succeeded"])
        self.assertEqual(payload["record"]["returncode"], 0)

    def test_execute_failure_returns_four_and_records(self) -> None:
        result = self.run_cli(
            "execute",
            "--task-id",
            "SM-CLI",
            "--",
            sys.executable,
            "-c",
            "raise SystemExit(9)",
        )

        self.assertEqual(result.returncode, 4, result.stderr)
        payload = json.loads(result.stdout)
        self.assertFalse(payload["record"]["succeeded"])
        self.assertEqual(payload["record"]["returncode"], 9)

    def test_execute_without_argv_returns_two(self) -> None:
        result = self.run_cli(
            "execute",
            "--task-id",
            "SM-CLI",
        )

        self.assertEqual(result.returncode, 2)
        self.assertIn("Execution argv must not be empty", result.stderr)


if __name__ == "__main__":
    unittest.main()
