"""Shared adapter behavior."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

from ..errors import InterfaceUnavailable
from ..models import CommandPreview
from ..policy import BuildAuthorityPolicy
from ..repository import require_isolated_worktree


class Adapter:
    """Base class for preview-only governed interfaces."""

    executable: str
    name: str

    def __init__(self, policy: BuildAuthorityPolicy) -> None:
        self.policy = policy

    def executable_path(self) -> str:
        path = shutil.which(self.executable)
        if path is None:
            raise InterfaceUnavailable(
                f"Required executable is unavailable: {self.executable}"
            )
        return path

    def version(self) -> str:
        result = subprocess.run(
            [self.executable_path(), "--version"],
            check=False,
            capture_output=True,
            text=True,
            timeout=30,
        )
        text = result.stdout.strip() or result.stderr.strip()
        if result.returncode != 0:
            raise InterfaceUnavailable(
                f"{self.executable} --version failed: {text}"
            )
        return text

    def preview(
        self,
        *,
        argv: tuple[str, ...],
        cwd: Path,
        mutating: bool,
        rationale: str,
    ) -> CommandPreview:
        self.policy.validate_argv(argv)

        if mutating:
            require_isolated_worktree(cwd)

        return CommandPreview.create(
            adapter=self.name,
            argv=(self.executable_path(), *argv),
            cwd=cwd.resolve(),
            mutating=mutating,
            requires_approval=mutating,
            rationale=rationale,
        )
