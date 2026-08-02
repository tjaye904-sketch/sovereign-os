"""Loading and enforcement of the Sovereign build authority policy."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any

from .errors import PolicyViolation


REQUIRED_TOP_LEVEL_KEYS = {
    "policy_id",
    "version",
    "human_authority",
    "agent_default_permissions",
    "agent_prohibited_actions",
    "repository_controls",
    "execution_controls",
    "integration_controls",
}


class BuildAuthorityPolicy:
    """Runtime representation of the build authority policy."""

    def __init__(self, document: dict[str, Any], source: Path) -> None:
        self.document = document
        self.source = source

    @classmethod
    def load(cls, path: Path) -> "BuildAuthorityPolicy":
        with path.open("r", encoding="utf-8") as handle:
            document = json.load(handle)

        missing = REQUIRED_TOP_LEVEL_KEYS - document.keys()
        if missing:
            raise PolicyViolation(
                "Authority policy is missing required keys: "
                + ", ".join(sorted(missing))
            )

        return cls(document=document, source=path.resolve())

    @property
    def prohibited_flags(self) -> frozenset[str]:
        controls = self.document["execution_controls"]
        return frozenset(controls["dangerous_bypass_flags"])

    def validate_argv(self, argv: tuple[str, ...]) -> None:
        used = self.prohibited_flags.intersection(argv)
        if used:
            raise PolicyViolation(
                "Command includes prohibited bypass flag(s): "
                + ", ".join(sorted(used))
            )

    def summary(self) -> dict[str, Any]:
        return {
            "policy_id": self.document["policy_id"],
            "version": self.document["version"],
            "human_principal": self.document["human_authority"]["principal"],
            "default_mode": self.document["execution_controls"]["default_mode"],
            "worktree_required": self.document["repository_controls"][
                "agent_worktree_required"
            ],
            "approval_required_for_mutation": self.document[
                "execution_controls"
            ]["approval_required_for_mutation"],
            "prohibited_flags": sorted(self.prohibited_flags),
        }
