"""Provider registration and construction."""

from __future__ import annotations

from collections.abc import Callable

from .base import ModelProvider
from .ollama import OllamaProvider

ProviderFactory = Callable[..., ModelProvider]


class UnknownProviderError(ValueError):
    """Raised when a provider name is not registered."""


_PROVIDER_FACTORIES: dict[str, ProviderFactory] = {
    "ollama": OllamaProvider,
}


def provider_names() -> tuple[str, ...]:
    """Return registered provider names in stable order."""

    return tuple(sorted(_PROVIDER_FACTORIES))


def get_provider(
    name: str,
    **options: object,
) -> ModelProvider:
    """Construct a registered provider by name."""

    normalized_name = name.strip().lower()
    if not normalized_name:
        raise ValueError("provider name must not be empty")

    try:
        factory = _PROVIDER_FACTORIES[normalized_name]
    except KeyError as error:
        available = ", ".join(provider_names())
        raise UnknownProviderError(
            f"Unknown provider {name!r}. Available providers: {available}"
        ) from error

    return factory(**options)
