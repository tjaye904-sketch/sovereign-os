from __future__ import annotations

import json
import unittest
from urllib.error import URLError
from unittest.mock import MagicMock, patch

from sovereign_mesh.providers import (
    OllamaProvider,
    ProviderResponseError,
    ProviderUnavailable,
)


def mocked_response(payload: object) -> MagicMock:
    response = MagicMock()
    response.read.return_value = json.dumps(payload).encode("utf-8")
    response.__enter__.return_value = response
    response.__exit__.return_value = False
    return response


class OllamaProviderTest(unittest.TestCase):
    @patch(
        "sovereign_mesh.providers.ollama.urlopen",
        side_effect=URLError("connection refused"),
    )
    def test_provider_unavailable(self, _: object) -> None:
        provider = OllamaProvider()

        self.assertFalse(provider.available())

        with self.assertRaises(ProviderUnavailable):
            provider.list_models()

    @patch("sovereign_mesh.providers.ollama.urlopen")
    def test_list_models(self, urlopen: MagicMock) -> None:
        urlopen.return_value = mocked_response(
            {
                "models": [
                    {"name": "qwen2.5:7b"},
                    {"name": "llama3.2:latest"},
                ]
            }
        )

        provider = OllamaProvider()

        self.assertEqual(
            provider.list_models(),
            ["qwen2.5:7b", "llama3.2:latest"],
        )

        request = urlopen.call_args.args[0]
        self.assertEqual(request.full_url, "http://localhost:11434/api/tags")
        self.assertEqual(request.get_method(), "GET")

    @patch("sovereign_mesh.providers.ollama.urlopen")
    def test_successful_generation(self, urlopen: MagicMock) -> None:
        urlopen.return_value = mocked_response(
            {
                "model": "qwen2.5:7b",
                "response": "Repository inspection complete.",
                "done": True,
                "total_duration": 123456,
            }
        )

        provider = OllamaProvider()
        response = provider.generate(
            model="qwen2.5:7b",
            prompt="Inspect the repository.",
            system="Return concise findings.",
        )

        self.assertEqual(response.provider, "ollama")
        self.assertEqual(response.model, "qwen2.5:7b")
        self.assertEqual(
            response.content,
            "Repository inspection complete.",
        )
        self.assertTrue(response.metadata["done"])

        request = urlopen.call_args.args[0]
        request_payload = json.loads(request.data.decode("utf-8"))

        self.assertEqual(
            request.full_url,
            "http://localhost:11434/api/generate",
        )
        self.assertEqual(request.get_method(), "POST")
        self.assertEqual(request_payload["model"], "qwen2.5:7b")
        self.assertEqual(
            request_payload["prompt"],
            "Inspect the repository.",
        )
        self.assertEqual(
            request_payload["system"],
            "Return concise findings.",
        )
        self.assertFalse(request_payload["stream"])
        self.assertEqual(
            request_payload["options"]["temperature"],
            0.0,
        )

    @patch(
        "sovereign_mesh.providers.ollama.urlopen",
        side_effect=TimeoutError("timed out"),
    )
    def test_timeout(self, _: object) -> None:
        provider = OllamaProvider(timeout_seconds=5)

        with self.assertRaisesRegex(
            ProviderUnavailable,
            "unavailable",
        ):
            provider.generate(
                model="qwen2.5:7b",
                prompt="Inspect the repository.",
            )

    @patch("sovereign_mesh.providers.ollama.urlopen")
    def test_malformed_generation_output(
        self,
        urlopen: MagicMock,
    ) -> None:
        response = MagicMock()
        response.read.return_value = b"not-json"
        response.__enter__.return_value = response
        response.__exit__.return_value = False
        urlopen.return_value = response

        provider = OllamaProvider()

        with self.assertRaises(ProviderResponseError):
            provider.generate(
                model="qwen2.5:7b",
                prompt="Inspect the repository.",
            )

    @patch("sovereign_mesh.providers.ollama.urlopen")
    def test_missing_response_content(
        self,
        urlopen: MagicMock,
    ) -> None:
        urlopen.return_value = mocked_response(
            {
                "model": "qwen2.5:7b",
                "done": True,
            }
        )

        provider = OllamaProvider()

        with self.assertRaisesRegex(
            ProviderResponseError,
            "textual output",
        ):
            provider.generate(
                model="qwen2.5:7b",
                prompt="Inspect the repository.",
            )


class ProviderRegistryTest(unittest.TestCase):
    def test_registered_provider_names(self) -> None:
        from sovereign_mesh.providers import provider_names

        self.assertEqual(provider_names(), ("ollama",))

    def test_get_provider(self) -> None:
        from sovereign_mesh.providers import (
            OllamaProvider,
            get_provider,
        )

        provider = get_provider(
            " OLLAMA ",
            base_url="http://127.0.0.1:11434/",
            timeout_seconds=30,
        )

        self.assertIsInstance(provider, OllamaProvider)
        self.assertEqual(
            provider.base_url,
            "http://127.0.0.1:11434",
        )
        self.assertEqual(provider.timeout_seconds, 30)

    def test_unknown_provider_is_rejected(self) -> None:
        from sovereign_mesh.providers import (
            UnknownProviderError,
            get_provider,
        )

        with self.assertRaisesRegex(
            UnknownProviderError,
            "Unknown provider",
        ):
            get_provider("missing")

    def test_empty_provider_name_is_rejected(self) -> None:
        from sovereign_mesh.providers import get_provider

        with self.assertRaisesRegex(
            ValueError,
            "must not be empty",
        ):
            get_provider("   ")

class ProviderServiceTest(unittest.TestCase):
    def test_resolves_configured_defaults(self) -> None:
        from sovereign_mesh.providers import (
            OllamaProvider,
            ProviderService,
        )

        service = ProviderService(
            default_provider="ollama",
            default_model="qwen2.5:7b",
            provider_options={
                "base_url": "http://127.0.0.1:11434/",
                "timeout_seconds": 30,
            },
        )

        provider = service.resolve_provider()

        self.assertIsInstance(provider, OllamaProvider)
        self.assertEqual(
            provider.base_url,
            "http://127.0.0.1:11434",
        )
        self.assertEqual(provider.timeout_seconds, 30)
        self.assertEqual(
            service.resolve_model(),
            "qwen2.5:7b",
        )

    def test_explicit_selection_overrides_defaults(self) -> None:
        from sovereign_mesh.providers import ProviderService

        service = ProviderService(
            default_provider="missing",
            default_model="default-model",
        )

        with patch(
            "sovereign_mesh.providers.service.get_provider"
        ) as get_provider_mock:
            provider = object()
            get_provider_mock.return_value = provider

            resolved = service.resolve_provider("ollama")

        self.assertIs(resolved, provider)
        get_provider_mock.assert_called_once_with("ollama")

        self.assertEqual(
            service.resolve_model("explicit-model"),
            "explicit-model",
        )

    def test_missing_provider_is_rejected(self) -> None:
        from sovereign_mesh.providers import (
            ProviderConfigurationError,
            ProviderService,
        )

        service = ProviderService(default_model="qwen2.5:7b")

        with self.assertRaisesRegex(
            ProviderConfigurationError,
            "No provider",
        ):
            service.resolve_provider()

    def test_missing_model_is_rejected(self) -> None:
        from sovereign_mesh.providers import (
            ProviderConfigurationError,
            ProviderService,
        )

        service = ProviderService(default_provider="ollama")

        with self.assertRaisesRegex(
            ProviderConfigurationError,
            "No model",
        ):
            service.resolve_model()

    def test_generate_delegates_to_selected_provider(self) -> None:
        from sovereign_mesh.models import CompletionResponse
        from sovereign_mesh.providers import ProviderService

        expected = CompletionResponse.create(
            provider="ollama",
            model="qwen2.5:7b",
            content="Inspection complete.",
        )

        provider = MagicMock()
        provider.generate.return_value = expected

        service = ProviderService(
            default_provider="ollama",
            default_model="qwen2.5:7b",
        )

        with patch(
            "sovereign_mesh.providers.service.get_provider",
            return_value=provider,
        ):
            response = service.generate(
                prompt="Inspect the repository.",
                system="Return concise findings.",
                temperature=0.2,
            )

        self.assertEqual(response, expected)
        provider.generate.assert_called_once_with(
            model="qwen2.5:7b",
            prompt="Inspect the repository.",
            system="Return concise findings.",
            temperature=0.2,
        )

    def test_empty_default_configuration_is_rejected(self) -> None:
        from sovereign_mesh.providers import (
            ProviderConfigurationError,
            ProviderService,
        )

        with self.assertRaisesRegex(
            ProviderConfigurationError,
            "default_provider must not be empty",
        ):
            ProviderService(default_provider="   ")


if __name__ == "__main__":
    unittest.main()
