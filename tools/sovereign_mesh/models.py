"""Core immutable records used by the Sovereign Development Mesh."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Mapping, Sequence


@dataclass(frozen=True)
class CommandPreview:
    """A command constructed by an adapter but not yet executed."""

    adapter: str
    argv: tuple[str, ...]
    cwd: Path
    mutating: bool
    requires_approval: bool
    rationale: str

    @classmethod
    def create(
        cls,
        *,
        adapter: str,
        argv: Sequence[str],
        cwd: Path,
        mutating: bool,
        requires_approval: bool,
        rationale: str,
    ) -> "CommandPreview":
        return cls(
            adapter=adapter,
            argv=tuple(argv),
            cwd=cwd,
            mutating=mutating,
            requires_approval=requires_approval,
            rationale=rationale,
        )


@dataclass(frozen=True)
class CompletionResponse:
    """Normalized text completion returned by a model provider."""

    provider: str
    model: str
    content: str
    metadata: Mapping[str, object]

    @classmethod
    def create(
        cls,
        *,
        provider: str,
        model: str,
        content: str,
        metadata: Mapping[str, object] | None = None,
    ) -> "CompletionResponse":
        return cls(
            provider=provider,
            model=model,
            content=content,
            metadata=dict(metadata or {}),
        )


@dataclass(frozen=True)
class RepositoryIdentity:
    """Identity information for a Git checkout."""

    path: Path
    git_common_dir: Path
    git_dir: Path
    top_level: Path
    branch: str
    head: str
    is_primary_checkout: bool


@dataclass(frozen=True)
class AgentProposal:
    """Structured proposal returned by an AI adapter."""

    adapter: str
    rationale: str
    commands: tuple[tuple[str, ...], ...]
    metadata: Mapping[str, str]

    @classmethod
    def create(
        cls,
        *,
        adapter: str,
        rationale: str,
        commands: Sequence[Sequence[str]],
        metadata: Mapping[str, str] | None = None,
    ) -> "AgentProposal":
        return cls(
            adapter=adapter,
            rationale=rationale,
            commands=tuple(tuple(command) for command in commands),
            metadata=dict(metadata or {}),
        )
