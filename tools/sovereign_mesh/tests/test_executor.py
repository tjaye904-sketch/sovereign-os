from __future__ import annotations

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from sovereign_mesh.executor import GovernedExecutor
from sovereign_mesh.workflow import ManagedWorkflow, WorkflowError


def git(path: Path, *arguments: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(path), *arguments],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return result.stdout.strip()


class GovernedExecutorTest(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.repository = (
            Path(self.temporary.name) / "repository"
        )
        self.repository.mkdir()

        git(self.repository, "init")
        git(
            self.repository,
            "config",
            "user.name",
            "Sovereign Test",
        )
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

        self.workflow = ManagedWorkflow(self.repository)
        self.task = self.workflow.create_task(
            task_id="SM-EXEC",
            title="Executor test",
            prompt="Run governed test commands.",
            adapter="codex",
        )
        self.executor = GovernedExecutor(self.workflow)

    def tearDown(self) -> None:
        subprocess.run(
            [
                "git",
                "-C",
                str(self.repository),
                "worktree",
                "prune",
            ],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        self.temporary.cleanup()

    def test_successful_execution_is_recorded(self) -> None:
        result = self.executor.execute(
            task_id="SM-EXEC",
            argv=(
                sys.executable,
                "-c",
                "print('governed execution')",
            ),
        )

        record = result["record"]

        self.assertTrue(record["succeeded"])
        self.assertEqual(record["returncode"], 0)
        self.assertFalse(record["timed_out"])
        self.assertTrue(
            Path(result["record_path"]).is_file()
        )
        self.assertIn(
            "governed execution",
            Path(record["stdout_path"]).read_text(
                encoding="utf-8"
            ),
        )
        self.assertEqual(
            result["evidence"]["task_id"],
            "SM-EXEC",
        )
        self.assertEqual(
            result["evidence"]["source_identity"],
            record["execution_digest"],
        )
        self.assertNotEqual(
            result["evidence"]["source_sha256"],
            record["execution_digest"],
        )
        self.assertEqual(
            result["evidence"]["command_preview"],
            record["command_display"],
        )
        self.assertTrue(
            Path(result["evidence_path"]).is_file()
        )

    def test_failed_execution_is_recorded(self) -> None:
        result = self.executor.execute(
            task_id="SM-EXEC",
            argv=(
                sys.executable,
                "-c",
                "raise SystemExit(7)",
            ),
        )

        self.assertFalse(result["record"]["succeeded"])
        self.assertEqual(result["record"]["returncode"], 7)
        self.assertTrue(
            Path(result["record_path"]).is_file()
        )
        self.assertEqual(
            result["evidence"]["task_id"],
            "SM-EXEC",
        )
        self.assertEqual(
            result["evidence"]["source_identity"],
            result["record"]["execution_digest"],
        )
        self.assertNotEqual(
            result["evidence"]["source_sha256"],
            result["record"]["execution_digest"],
        )

    def test_timeout_is_recorded(self) -> None:
        result = self.executor.execute(
            task_id="SM-EXEC",
            argv=(
                sys.executable,
                "-c",
                "import time; time.sleep(2)",
            ),
            timeout_seconds=1,
        )

        self.assertTrue(result["record"]["timed_out"])
        self.assertEqual(result["record"]["returncode"], 124)
        self.assertFalse(result["record"]["succeeded"])

    def test_execution_cwd_cannot_escape_worktree(self) -> None:
        with self.assertRaises(WorkflowError):
            self.executor.execute(
                task_id="SM-EXEC",
                argv=(
                    sys.executable,
                    "-c",
                    "print('unreachable')",
                ),
                cwd_relative="..",
            )

    def test_empty_argv_is_rejected(self) -> None:
        with self.assertRaises(WorkflowError):
            self.executor.execute(
                task_id="SM-EXEC",
                argv=(),
            )


if __name__ == "__main__":
    unittest.main()
