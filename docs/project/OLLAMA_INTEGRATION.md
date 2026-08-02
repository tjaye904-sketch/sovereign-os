# Ollama Integration Notes

**Owner model:** Hermes Nemotron 3 Supercloud (local Ollama)

## Configuration

| Env | Meaning | Default |
|-----|---------|---------|
| `OLLAMA_HOST` | Base URL | `http://127.0.0.1:11434` |
| `SOVEREIGN_OLLAMA_MODEL` | Chat model name | `openclaw` |

Adjust `SOVEREIGN_OLLAMA_MODEL` to match `ollama list` exactly if the tag differs
(e.g. `openclaw`).

## CLI

```bash
export OLLAMA_HOST=http://127.0.0.1:11434
export SOVEREIGN_OLLAMA_MODEL='<exact-name-from-ollama-list>'
./sovereign-cli models
./sovereign-cli chat "Reply with PONG"
./sovereign-cli chat-tools "Store a note about batteries"
```

## Tool calling

`chat-tools` advertises a `record_memory` tool. The router returns `tool_calls`;
Sovereign does **not** auto-execute tools without Governance for consequential
actions. Wiring tool results back into a second chat turn is the next increment.

## Sandbox limitation

The engineering sandbox may not share network with the host Ollama process.
Live tests **SKIP** (exit 0) when Ollama is unreachable. Run CLI/tests on the
same machine as Ollama for live verification.
