"""Local Ollama HTTP API provider."""

from __future__ import annotations

import json
from typing import Any
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen

from ..models import CompletionResponse
from .base import (
    ModelProvider,
    ProviderResponseError,
    ProviderUnavailable,
)


class OllamaProvider(ModelProvider):
    """Invoke models through the local Ollama HTTP API."""

    name = "ollama"

    def __init__(
        self,
        *,
        base_url: str = "http://localhost:11434",
        timeout_seconds: int = 120,
    ) -> None:
        if timeout_seconds < 1:
            raise ValueError("timeout_seconds must be at least 1")

        normalized_url = base_url.rstrip("/")
        if not normalized_url:
            raise ValueError("base_url must not be empty")

        self.base_url = normalized_url
        self.timeout_seconds = timeout_seconds

    def available(self) -> bool:
        try:
            self._request("GET", "/api/tags")
        except ProviderUnavailable:
            return False
        return True

    def list_models(self) -> list[str]:
        payload = self._request("GET", "/api/tags")

        models = payload.get("models")
        if not isinstance(models, list):
            raise ProviderResponseError(
                "Ollama model-list response does not contain a models array."
            )

        names: list[str] = []
        for entry in models:
            if not isinstance(entry, dict):
                raise ProviderResponseError(
                    "Ollama model-list entry must be a JSON object."
                )

            name = entry.get("name")
            if not isinstance(name, str) or not name:
                raise ProviderResponseError(
                    "Ollama model-list entry does not contain a valid name."
                )

            names.append(name)

        return names

    def generate(
        self,
        *,
        model: str,
        prompt: str,
        system: str | None = None,
        temperature: float = 0.0,
    ) -> CompletionResponse:
        if not model.strip():
            raise ValueError("model must not be empty")
        if not prompt.strip():
            raise ValueError("prompt must not be empty")
        if temperature < 0.0:
            raise ValueError("temperature must not be negative")

        request_payload: dict[str, Any] = {
            "model": model,
            "prompt": prompt,
            "stream": False,
            "options": {
                "temperature": temperature,
            },
        }

        if system is not None:
            request_payload["system"] = system

        payload = self._request(
            "POST",
            "/api/generate",
            payload=request_payload,
        )

        content = payload.get("response")
        if not isinstance(content, str):
            raise ProviderResponseError(
                "Ollama response does not contain textual output."
            )

        response_model = payload.get("model", model)
        if not isinstance(response_model, str):
            response_model = model

        metadata = {
            key: value
            for key, value in payload.items()
            if key not in {"model", "response"}
        }

        return CompletionResponse.create(
            provider=self.name,
            model=response_model,
            content=content,
            metadata=metadata,
        )

    def _request(
        self,
        method: str,
        path: str,
        *,
        payload: dict[str, Any] | None = None,
    ) -> dict[str, Any]:
        body = None
        headers = {
            "Accept": "application/json",
        }

        if payload is not None:
            body = json.dumps(payload).encode("utf-8")
            headers["Content-Type"] = "application/json"

        request = Request(
            f"{self.base_url}{path}",
            data=body,
            headers=headers,
            method=method,
        )

        try:
            with urlopen(
                request,
                timeout=self.timeout_seconds,
            ) as response:
                raw_body = response.read()
        except HTTPError as error:
            detail = error.read().decode(
                "utf-8",
                errors="replace",
            ).strip()
            raise ProviderUnavailable(
                f"Ollama API request failed with HTTP "
                f"{error.code}: {detail}"
            ) from error
        except (URLError, TimeoutError, OSError) as error:
            raise ProviderUnavailable(
                f"Ollama API is unavailable: {error}"
            ) from error

        try:
            decoded = json.loads(raw_body.decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError) as error:
            raise ProviderResponseError(
                "Ollama returned malformed JSON."
            ) from error

        if not isinstance(decoded, dict):
            raise ProviderResponseError(
                "Ollama response must be a JSON object."
            )

        return decoded
