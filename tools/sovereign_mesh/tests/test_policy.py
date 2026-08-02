from __future__ import annotations

import unittest
from pathlib import Path

from sovereign_mesh.errors import PolicyViolation
from sovereign_mesh.policy import BuildAuthorityPolicy


PACKAGE_ROOT = Path(__file__).resolve().parents[1]


class BuildAuthorityPolicyTest(unittest.TestCase):
    def setUp(self) -> None:
        self.policy = BuildAuthorityPolicy.load(
            PACKAGE_ROOT / "policy" / "build-authority.json"
        )

    def test_policy_loads(self) -> None:
        summary = self.policy.summary()
        self.assertEqual(
            summary["policy_id"],
            "sovereign-build-authority-v1",
        )
        self.assertEqual(summary["default_mode"], "preview")
        self.assertTrue(summary["worktree_required"])

    def test_dangerous_codex_flag_is_rejected(self) -> None:
        with self.assertRaises(PolicyViolation):
            self.policy.validate_argv(
                (
                    "exec",
                    "--dangerously-bypass-approvals-and-sandbox",
                )
            )

    def test_hermes_yolo_flag_is_rejected(self) -> None:
        with self.assertRaises(PolicyViolation):
            self.policy.validate_argv(("--yolo",))


if __name__ == "__main__":
    unittest.main()
