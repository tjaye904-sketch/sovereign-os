"""Provider selection and completion service."""

from __future__ import annotations

from collections.abc import Mapping

from ..models import CompletionResponse
from .base import ModelProvider
from .registry import get_provider


class ProviderConfigurationError(ValueError):
    """Raised when provider or model configuration is incomplete."""


class ProviderService:
    """Resolve providers and delegate normalized text generation."""

    def __init__(
        self,
        *,
        default_provider: str | None = None,
        default_model: str | None = None,
        provider_options: Mapping[str, object] | None = None,
    ) -> None:
        self.default_provider = self._normalize_optional(
            default_provider,
            field="default_provider",
        )
        self.default_model = self._normalize_optional(
            default_model,
            field="default_model",
        )
        self.provider_options = dict(provider_options or {})

    def resolve_provider(
        self,
        provider: str | None = None,
    ) -> ModelProvider:
        """Construct the selected provider."""

        provider_name = self._select_required(
            explicit=provider,
            default=self.default_provider,
            field="provider",
        )

        return get_provider(
            provider_name,
            **self.provider_options,
        )

    def resolve_model(
        self,
        model: str | None = None,
    ) -> str:
        """Return the explicit or configured model name."""

        return self._select_required(
            explicit=model,
            default=self.default_model,
            field="model",
        )

    def generate(
        self,
        *,
        prompt: str,
        provider: str | None = None,
        model: str | None = None,
        system: str | None = None,
        temperature: float = 0.0,
    ) -> CompletionResponse:
        """Generate a completion using explicit or configured defaults."""

        selected_provider = self.resolve_provider(provider)
        selected_model = self.resolve_model(model)

        return selected_provider.generate(
            model=selected_model,
            prompt=prompt,
            system=system,
            temperature=temperature,
        )

    @staticmethod
    def _normalize_optional(
        value: str | None,
        *,
        field: str,
    ) -> str | None:
        if value is None:
            return None

        normalized = value.strip()
        if not normalized:
            raise ProviderConfigurationError(
                f"{field} must not be empty"
            )

        return normalized

    @classmethod
    def _select_required(
        cls,
        *,
        explicit: str | None,
        default: str | None,
        field: str,
    ) -> str:
        selected = cls._normalize_optional(
            explicit,
            field=field,
        )
        if selected is not None:
            return selected

        if default is not None:
            return default

        raise ProviderConfigurationError(
            f"No {field} was provided or configured."
        )
