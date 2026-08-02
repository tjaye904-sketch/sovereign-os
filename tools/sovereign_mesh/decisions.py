"""Human decision record creation and verification."""

from __future__ import annotations

import hashlib
import json
import uuid
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

from .errors import ApprovalRequired


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as handle:
        value = json.load(handle)
    if not isinstance(value, dict):
        raise ValueError(f"Expected a JSON object: {path}")
    return value


def record_decision(
    *,
    proposal_path: Path,
    decision_dir: Path,
    decision: str,
    notes: str,
) -> Path:
    allowed = {"approved", "rejected", "revision_requested"}
    if decision not in allowed:
        raise ValueError(
            f"Unsupported decision {decision!r}; expected one of {sorted(allowed)}"
        )

    proposal = load_json(proposal_path)
    proposal_id = proposal.get("proposal_id")
    if not isinstance(proposal_id, str) or not proposal_id:
        raise ValueError("Proposal is missing a valid proposal_id")

    record = {
        "decision_id": f"decision-{uuid.uuid4()}",
        "proposal_id": proposal_id,
        "authority": "sovereign-owner",
        "decision": decision,
        "recorded_at": datetime.now(timezone.utc).isoformat(),
        "proposal_sha256": sha256_file(proposal_path),
        "notes": notes,
    }

    decision_dir.mkdir(parents=True, exist_ok=True)
    output = decision_dir / f"{proposal_id}.decision.json"

    if output.exists():
        raise FileExistsError(
            f"Decision already exists for proposal {proposal_id}: {output}"
        )

    with output.open("x", encoding="utf-8") as handle:
        json.dump(record, handle, indent=2, sort_keys=True)
        handle.write("\n")

    return output


def require_approved_decision(
    *,
    proposal_path: Path,
    decision_path: Path,
) -> dict[str, Any]:
    proposal = load_json(proposal_path)
    decision = load_json(decision_path)

    if decision.get("decision") != "approved":
        raise ApprovalRequired(
            f"Proposal decision is not approved: {decision.get('decision')}"
        )

    if decision.get("authority") != "sovereign-owner":
        raise ApprovalRequired("Decision authority is not sovereign-owner")

    if decision.get("proposal_id") != proposal.get("proposal_id"):
        raise ApprovalRequired("Decision does not reference this proposal")

    current_digest = sha256_file(proposal_path)
    if decision.get("proposal_sha256") != current_digest:
        raise ApprovalRequired(
            "Proposal changed after the human decision was recorded"
        )

    return decision
