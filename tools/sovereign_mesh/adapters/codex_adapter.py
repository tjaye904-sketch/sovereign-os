"""Codex CLI command construction."""

from __future__ import annotations

from pathlib import Path

from .base import Adapter
from ..models import CommandPreview


class CodexAdapter(Adapter):
    executable = "codex"
    name = "codex"

    def inspect(self, worktree: Path, prompt: str) -> CommandPreview:
        return self.preview(
            argv=(
                "exec",
                "--ephemeral",
                "--sandbox",
                "read-only",
                "--ask-for-approval",
                "never",
                "--cd",
                str(worktree.resolve()),
                prompt,
            ),
            cwd=worktree,
            mutating=False,
            rationale="Read-only Codex repository inspection.",
        )

    def implement(self, worktree: Path, prompt: str) -> CommandPreview:
        return self.preview(
            argv=(
                "exec",
                "--ephemeral",
                "--sandbox",
                "workspace-write",
                "--ask-for-approval",
                "never",
                "--cd",
                str(worktree.resolve()),
                prompt,
            ),
            cwd=worktree,
            mutating=True,
            rationale="Codex implementation inside an isolated worktree.",
        )

    def review(
        self,
        worktree: Path,
        *,
        base_branch: str,
        prompt: str,
    ) -> CommandPreview:
        return self.preview(
            argv=(
                "review",
                "--base",
                base_branch,
                prompt,
            ),
            cwd=worktree,
            mutating=False,
            rationale="Independent Codex review of isolated worktree changes.",
        )
