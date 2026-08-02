# Building sovereign-cli on your host (Pop!_OS / Ubuntu)

The engineering sandbox is isolated from your Ollama process. Build and run
**on the same machine** where `ollama` and Hermes dashboard run.

## Dependencies

```bash
sudo apt install -y g++ libargon2-dev
# ollama already running with hermes nemotron model
```

## Build

From the `sovereign-work` tree:

```bash
chmod +x scripts/build-sovereign-cli.sh
./scripts/build-sovereign-cli.sh
# binary: ./build/sovereign-cli
```

## Configure model name

```bash
ollama list   # copy exact NAME column
export OLLAMA_HOST=http://127.0.0.1:11434
export SOVEREIGN_OLLAMA_MODEL='<exact-name>'
# example if list shows openclaw:latest
```

## Smoke

```bash
./build/sovereign-cli status
./build/sovereign-cli models
./build/sovereign-cli bootstrap owner 'your-secret'
./build/sovereign-cli login owner 'your-secret'
./build/sovereign-cli chat "Reply with PONG"
./build/sovereign-cli recommend deploy "ship"
# approve <id> then:
./build/sovereign-cli execute deploy <decision-hex>
```

Hermes dashboard (`http://localhost:9119`) is separate from Sovereign; it does
not replace sovereign-cli.
