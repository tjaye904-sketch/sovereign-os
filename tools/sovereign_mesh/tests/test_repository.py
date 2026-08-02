from __future__ import annotations

import subprocess
import tempfile
import unittest
from pathlib import Path

from sovereign_mesh.errors import PrimaryCheckoutViolation
from sovereign_mesh.repository import (
    inspect_repository,
    require_isolated_worktree,
)


class RepositoryIsolationTest(unittest.TestCase):
    def run_git(self, path: Path, *args: str) -> None:
        subprocess.run(
            ["git", "-C", str(path), *args],
            check=True,
            capture_output=True,
            text=True,
        )

    def test_primary_checkout_is_rejected_and_worktree_is_allowed(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "repo"
            worktree = Path(temporary) / "worktree"
            root.mkdir()

            self.run_git(root, "init", "-b", "master")
            self.run_git(root, "config", "user.name", "Mesh Test")
            self.run_git(
                root,
                "config",
                "user.email",
                "mesh-test@example.invalid",
            )

            (root / "README.md").write_text(
                "test\n",
                encoding="utf-8",
            )
            self.run_git(root, "add", "README.md")
            self.run_git(root, "commit", "-m", "initial")

            primary = inspect_repository(root)
            self.assertTrue(primary.is_primary_checkout)

            with self.assertRaises(PrimaryCheckoutViolation):
                require_isolated_worktree(root)

            self.run_git(
                root,
                "worktree",
                "add",
                "-b",
                "agent/test",
                str(worktree),
            )

            isolated = require_isolated_worktree(worktree)
            self.assertFalse(isolated.is_primary_checkout)
            self.assertEqual(isolated.branch, "agent/test")


if __name__ == "__main__":
    unittest.main()
