from __future__ import annotations

import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from sovereign_mesh.executor import GovernedExecutor
from sovereign_mesh.workflow import ManagedWorkflow, WorkflowError, sha256_payload


def git(path: Path, *arguments: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(path), *arguments],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return result.stdout.strip()


class GovernedExecutionProposalTest(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.repository = Path(self.temporary.name) / "repository"
        self.repository.mkdir()

        git(self.repository, "init")
        git(self.repository, "config", "user.name", "Sovereign Test")
        git(self.repository, "config", "user.email", "sovereign@example.invalid")

        (self.repository / "README.md").write_text(
            "Sovereign test repository\n",
            encoding="utf-8",
        )
        git(self.repository, "add", "README.md")
        git(self.repository, "commit", "-m", "initial")

        self.workflow = ManagedWorkflow(self.repository)
        self.task = self.workflow.create_task(
            task_id="SM-PROPOSAL",
            title="Proposal execution binding",
            prompt="Produce governed execution evidence.",
            adapter="codex",
        )
        self.executor = GovernedExecutor(self.workflow)

    def tearDown(self) -> None:
        subprocess.run(
            ["git", "-C", str(self.repository), "worktree", "prune"],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        self.temporary.cleanup()

    def test_execution_evidence_is_embedded_in_proposal(self) -> None:
        result = self.executor.execute(
            task_id="SM-PROPOSAL",
            argv=(sys.executable, "-c", "print('proposal evidence')"),
        )

        proposal = self.workflow.assemble_proposal(
            task_id="SM-PROPOSAL",
            summary="Governed execution completed.",
            recommendation="Review for integration.",
        )

        self.assertIsNotNone(proposal["execution"])
        self.assertEqual(
            proposal["execution"]["execution_digest"],
            result["record"]["execution_digest"],
        )
        self.assertEqual(
            proposal["execution"]["execution_id"],
            result["record"]["execution_id"],
        )
        self.assertTrue(proposal["execution"]["succeeded"])
        self.assertEqual(proposal["execution"]["returncode"], 0)

    def test_tampered_execution_record_is_rejected(self) -> None:
        self.executor.execute(
            task_id="SM-PROPOSAL",
            argv=(sys.executable, "-c", "print('original')"),
        )

        evidence_path = self.workflow.evidence_path("SM-PROPOSAL")
        evidence = json.loads(evidence_path.read_text(encoding="utf-8"))
        execution = json.loads(evidence["content"])
        execution["returncode"] = 99
        evidence["content"] = json.dumps(
            execution,
            indent=2,
            sort_keys=True,
        ) + "\n"

        evidence_body = dict(evidence)
        evidence_body.pop("evidence_digest", None)
        evidence["evidence_digest"] = sha256_payload(evidence_body)
        evidence_path.write_text(
            json.dumps(evidence, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )

        with self.assertRaisesRegex(WorkflowError, "execution record integrity"):
            self.workflow.assemble_proposal(
                task_id="SM-PROPOSAL",
                summary="Tampered execution.",
                recommendation="Reject.",
            )

    def test_manual_evidence_remains_compatible(self) -> None:
        source = Path(self.task["worktree"]) / "manual.txt"
        source.write_text("Manual review completed.\n", encoding="utf-8")
        self.workflow.record_evidence(
            task_id="SM-PROPOSAL",
            source_file=source,
            producer="human",
            command_preview="manual review",
        )

        proposal = self.workflow.assemble_proposal(
            task_id="SM-PROPOSAL",
            summary="Manual evidence reviewed.",
            recommendation="Continue review.",
        )

        self.assertIsNone(proposal["execution"])


if __name__ == "__main__":
    unittest.main()
