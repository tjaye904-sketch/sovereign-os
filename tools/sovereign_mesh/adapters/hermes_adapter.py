"""Hermes Agent command construction."""

from __future__ import annotations

from pathlib import Path

from .base import Adapter
from ..models import CommandPreview


class HermesAdapter(Adapter):
    executable = "hermes"
    name = "hermes"

    def inspect(self, worktree: Path, prompt: str) -> CommandPreview:
        return self.preview(
            argv=(
                "--safe-mode",
                "--ignore-rules",
                "--oneshot",
                prompt,
            ),
            cwd=worktree,
            mutating=False,
            rationale=(
                "Restricted Hermes inspection. The external mesh executor must "
                "supply an operating-system-level read-only boundary because "
                "Hermes oneshot bypasses internal approvals."
            ),
        )

    def implement(self, worktree: Path, prompt: str) -> CommandPreview:
        return self.preview(
            argv=(
                "--worktree",
                "--ignore-rules",
                "--oneshot",
                prompt,
            ),
            cwd=worktree,
            mutating=True,
            rationale=(
                "Hermes implementation in an isolated worktree. Execution "
                "remains disabled until an external confinement mechanism is "
                "implemented and verified."
            ),
        )
