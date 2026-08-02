from __future__ import annotations

import json
import subprocess
import tempfile
import unittest
from pathlib import Path

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


class ManagedWorkflowTest(unittest.TestCase):
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

        self.workflow = ManagedWorkflow(self.repository)

    def tearDown(self) -> None:
        subprocess.run(
            ["git", "-C", str(self.repository), "worktree", "prune"],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        self.temporary.cleanup()

    def create_task(self, task_id: str = "SM-TEST") -> dict:
        return self.workflow.create_task(
            task_id=task_id,
            title="Test task",
            prompt="Inspect the repository.",
            adapter="codex",
        )

    def test_create_task_uses_isolated_worktree(self) -> None:
        task = self.create_task()
        inspection = self.workflow.inspect_task("SM-TEST")

        self.assertTrue(Path(task["worktree"]).is_dir())
        self.assertFalse(
            inspection["worktree"]["is_primary_checkout"]
        )
        self.assertEqual(
            task["base_commit"],
            git(self.repository, "rev-parse", "HEAD"),
        )

    def test_duplicate_task_is_rejected(self) -> None:
        self.create_task()

        with self.assertRaises(WorkflowError):
            self.create_task()

    def test_task_tampering_is_detected(self) -> None:
        self.create_task()
        task_path = self.workflow.task_path("SM-TEST")
        task = json.loads(task_path.read_text(encoding="utf-8"))
        task["prompt"] = "Tampered prompt"
        task_path.write_text(
            json.dumps(task),
            encoding="utf-8",
        )

        inspection = self.workflow.inspect_task("SM-TEST")

        self.assertFalse(
            inspection["integrity"]["task_digest_valid"]
        )

    def test_evidence_and_proposal_are_digest_bound(self) -> None:
        task = self.create_task()
        evidence_source = Path(task["worktree"]) / "evidence.txt"
        evidence_source.write_text(
            "No repository changes required.\n",
            encoding="utf-8",
        )

        evidence = self.workflow.record_evidence(
            task_id="SM-TEST",
            source_file=evidence_source,
            producer="test-agent",
            command_preview="agent inspect --read-only",
        )

        proposal = self.workflow.assemble_proposal(
            task_id="SM-TEST",
            summary="Inspection completed.",
            recommendation="No change.",
        )

        self.assertEqual(
            proposal["evidence_digest"],
            evidence["evidence_digest"],
        )
        self.assertEqual(
            proposal["task_digest"],
            task["task_digest"],
        )

    def test_unapproved_proposal_cannot_integrate(self) -> None:
        task = self.create_task()
        source = Path(task["worktree"]) / "evidence.txt"
        source.write_text(
            "Inspection result.\n",
            encoding="utf-8",
        )

        self.workflow.record_evidence(
            task_id="SM-TEST",
            source_file=source,
            producer="test-agent",
            command_preview="agent inspect --read-only",
        )

        proposal = self.workflow.assemble_proposal(
            task_id="SM-TEST",
            summary="Inspection complete.",
            recommendation="Review.",
        )

        decision = {
            "schema_version": 1,
            "proposal_digest": proposal["proposal_digest"],
            "decision": "rejected",
            "principal": "sovereign-owner",
            "notes": "Not approved.",
        }

        decision_path = self.workflow.decision_path("SM-TEST")
        decision_path.parent.mkdir(parents=True, exist_ok=True)
        decision_path.write_text(
            json.dumps(decision),
            encoding="utf-8",
        )

        authority = self.workflow.verify_integration_authority(
            "SM-TEST"
        )

        self.assertFalse(authority["authorized"])
        self.assertIsNone(
            authority["integration_command_preview"]
        )

    def test_approved_bound_proposal_authorizes_preview_only(self) -> None:
        task = self.create_task()
        source = Path(task["worktree"]) / "evidence.txt"
        source.write_text(
            "Inspection result.\n",
            encoding="utf-8",
        )

        self.workflow.record_evidence(
            task_id="SM-TEST",
            source_file=source,
            producer="test-agent",
            command_preview="agent inspect --read-only",
        )

        proposal = self.workflow.assemble_proposal(
            task_id="SM-TEST",
            summary="Inspection complete.",
            recommendation="Integrate.",
        )

        decision = {
            "schema_version": 1,
            "proposal_digest": proposal["proposal_digest"],
            "decision": "approved",
            "principal": "sovereign-owner",
            "notes": "Approved for integration.",
        }

        decision_path = self.workflow.decision_path("SM-TEST")
        decision_path.parent.mkdir(parents=True, exist_ok=True)
        decision_path.write_text(
            json.dumps(decision),
            encoding="utf-8",
        )

        authority = self.workflow.verify_integration_authority(
            "SM-TEST"
        )

        self.assertTrue(authority["authorized"])
        self.assertIn(
            "merge --no-ff sovereign-mesh/SM-TEST",
            authority["integration_command_preview"],
        )


if __name__ == "__main__":
    unittest.main()
