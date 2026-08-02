"""Git repository identity and isolation enforcement."""

from __future__ import annotations

import subprocess
from pathlib import Path

from .errors import InterfaceUnavailable, PrimaryCheckoutViolation
from .models import RepositoryIdentity


def _git(path: Path, *args: str) -> str:
    result = subprocess.run(
        ["git", "-C", str(path), *args],
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        message = result.stderr.strip() or result.stdout.strip()
        raise InterfaceUnavailable(f"git {' '.join(args)} failed: {message}")
    return result.stdout.strip()


def inspect_repository(path: Path) -> RepositoryIdentity:
    resolved = path.resolve()

    top_level = Path(
        _git(resolved, "rev-parse", "--show-toplevel")
    ).resolve()

    git_dir_text = _git(resolved, "rev-parse", "--absolute-git-dir")
    git_dir = Path(git_dir_text).resolve()

    common_dir_text = _git(resolved, "rev-parse", "--git-common-dir")
    common_dir_candidate = Path(common_dir_text)
    if not common_dir_candidate.is_absolute():
        common_dir_candidate = top_level / common_dir_candidate
    git_common_dir = common_dir_candidate.resolve()

    branch = _git(resolved, "branch", "--show-current")
    head = _git(resolved, "rev-parse", "HEAD")

    # In the primary checkout, the checkout's git directory is the common
    # repository directory. Linked worktrees receive a distinct git directory
    # beneath <common-dir>/worktrees/.
    is_primary = git_dir == git_common_dir

    return RepositoryIdentity(
        path=resolved,
        git_common_dir=git_common_dir,
        git_dir=git_dir,
        top_level=top_level,
        branch=branch,
        head=head,
        is_primary_checkout=is_primary,
    )


def require_isolated_worktree(path: Path) -> RepositoryIdentity:
    identity = inspect_repository(path)
    if identity.is_primary_checkout:
        raise PrimaryCheckoutViolation(
            "Agent execution against the primary Sovereign checkout is "
            "prohibited. Create or select an isolated Git worktree."
        )
    return identity
