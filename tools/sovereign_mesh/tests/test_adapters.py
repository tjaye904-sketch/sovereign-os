from __future__ import annotations

import unittest
from pathlib import Path
from unittest.mock import patch

from sovereign_mesh.adapters import (
    CodexAdapter,
    HermesAdapter,
    OpenClawAdapter,
)
from sovereign_mesh.errors import PrimaryCheckoutViolation
from sovereign_mesh.policy import BuildAuthorityPolicy


PACKAGE_ROOT = Path(__file__).resolve().parents[1]


class AdapterPreviewTest(unittest.TestCase):
    def setUp(self) -> None:
        self.policy = BuildAuthorityPolicy.load(
            PACKAGE_ROOT / "policy" / "build-authority.json"
        )

    @patch(
        "sovereign_mesh.adapters.base.Adapter.executable_path",
        return_value="/usr/bin/fake-agent",
    )
    def test_codex_read_only_preview(self, _: object) -> None:
        adapter = CodexAdapter(self.policy)
        preview = adapter.inspect(Path("/tmp"), "Inspect repository")
        self.assertFalse(preview.mutating)
        self.assertIn("read-only", preview.argv)

    @patch(
        "sovereign_mesh.adapters.base.Adapter.executable_path",
        return_value="/usr/bin/fake-agent",
    )
    @patch(
        "sovereign_mesh.adapters.base.require_isolated_worktree",
        side_effect=PrimaryCheckoutViolation("primary checkout"),
    )
    def test_mutation_requires_worktree(
        self,
        _: object,
        __: object,
    ) -> None:
        adapter = CodexAdapter(self.policy)
        with self.assertRaises(PrimaryCheckoutViolation):
            adapter.implement(Path("/tmp"), "Modify repository")

    @patch(
        "sovereign_mesh.adapters.base.Adapter.executable_path",
        return_value="/usr/bin/fake-agent",
    )
    def test_hermes_preview_never_uses_yolo(self, _: object) -> None:
        adapter = HermesAdapter(self.policy)
        preview = adapter.inspect(Path("/tmp"), "Inspect repository")
        self.assertNotIn("--yolo", preview.argv)

    @patch(
        "sovereign_mesh.adapters.base.Adapter.executable_path",
        return_value="/usr/bin/fake-agent",
    )
    def test_openclaw_audit_is_non_mutating(self, _: object) -> None:
        adapter = OpenClawAdapter(self.policy)
        preview = adapter.audit(
            Path("/tmp"),
            agent_id="reviewer",
        )
        self.assertFalse(preview.mutating)
        self.assertIn("--json", preview.argv)


if __name__ == "__main__":
    unittest.main()
