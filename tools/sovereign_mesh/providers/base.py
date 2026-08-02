"""Shared model-provider interfaces and errors."""

from __future__ import annotations

from abc import ABC, abstractmethod

from ..models import CompletionResponse


class ProviderError(RuntimeError):
    """Base error raised by model providers."""


class ProviderUnavailable(ProviderError):
    """Raised when a model provider cannot be reached or executed."""


class ProviderResponseError(ProviderError):
    """Raised when a provider returns an unusable response."""


class ModelProvider(ABC):
    """Abstract interface for local or remote language-model providers."""

    name: str

    @abstractmethod
    def available(self) -> bool:
        """Return whether the provider executable is available."""

    @abstractmethod
    def list_models(self) -> list[str]:
        """Return the names of models available from the provider."""

    @abstractmethod
    def generate(
        self,
        *,
        model: str,
        prompt: str,
        system: str | None = None,
        temperature: float = 0.0,
    ) -> CompletionResponse:
        """Generate one normalized completion."""
