from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from sovereign_mesh.decisions import (
    record_decision,
    require_approved_decision,
)
from sovereign_mesh.errors import ApprovalRequired


class DecisionRecordTest(unittest.TestCase):
    def write_proposal(self, root: Path) -> Path:
        proposal = root / "proposal.json"
        proposal.write_text(
            json.dumps(
                {
                    "proposal_id": "proposal-SDM-0001",
                    "task_id": "SDM-0001",
                    "base_commit": "a" * 40,
                    "summary": "Test proposal",
                    "evidence_ids": ["evidence-1"],
                    "reviews": [
                        {
                            "reviewer": "codex",
                            "verdict": "approve",
                        }
                    ],
                    "test_status": "passed",
                    "state": "awaiting_user_decision",
                },
                indent=2,
            )
            + "\n",
            encoding="utf-8",
        )
        return proposal

    def test_approved_unchanged_proposal_verifies(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            proposal = self.write_proposal(root)

            decision = record_decision(
                proposal_path=proposal,
                decision_dir=root / "decisions",
                decision="approved",
                notes="Approved by test owner.",
            )

            verified = require_approved_decision(
                proposal_path=proposal,
                decision_path=decision,
            )
            self.assertEqual(verified["decision"], "approved")

    def test_modified_proposal_invalidates_decision(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            proposal = self.write_proposal(root)

            decision = record_decision(
                proposal_path=proposal,
                decision_dir=root / "decisions",
                decision="approved",
                notes="Approved before mutation.",
            )

            proposal.write_text(
                proposal.read_text(encoding="utf-8") + " ",
                encoding="utf-8",
            )

            with self.assertRaises(ApprovalRequired):
                require_approved_decision(
                    proposal_path=proposal,
                    decision_path=decision,
                )


if __name__ == "__main__":
    unittest.main()
