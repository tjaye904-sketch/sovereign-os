"""OpenClaw Gateway command construction."""

from __future__ import annotations

from pathlib import Path

from .base import Adapter
from ..models import CommandPreview


class OpenClawAdapter(Adapter):
    executable = "openclaw"
    name = "openclaw"

    def agent_turn(
        self,
        worktree: Path,
        *,
        agent_id: str,
        session_key: str,
        prompt_file: Path,
        model: str | None = None,
    ) -> CommandPreview:
        argv: list[str] = [
            "agent",
            "--agent",
            agent_id,
            "--session-key",
            session_key,
            "--message-file",
            str(prompt_file.resolve()),
            "--json",
        ]

        if model:
            argv.extend(["--model", model])

        return self.preview(
            argv=tuple(argv),
            cwd=worktree,
            mutating=False,
            rationale=(
                "OpenClaw agent reasoning turn. Code mutation must be delegated "
                "through a separately governed isolated-worktree executor."
            ),
        )

    def audit(
        self,
        worktree: Path,
        *,
        agent_id: str,
        limit: int = 100,
    ) -> CommandPreview:
        return self.preview(
            argv=(
                "audit",
                "--agent",
                agent_id,
                "--limit",
                str(limit),
                "--json",
            ),
            cwd=worktree,
            mutating=False,
            rationale="Retrieve metadata-only OpenClaw agent audit evidence.",
        )
