#!/usr/bin/env bash
set -euo pipefail
BIN="$HOME/.local/bin"
mkdir -p "$BIN"
write_shim() {
  local name="$1" target="$2"
  cat > "$BIN/$name" << S
#!/usr/bin/env bash
export PATH="\$HOME/.local/bin:\$PATH"
exec sovereign-cognitive-cli $target "\$@"
S
  chmod +x "$BIN/$name"
}
write_shim sovereign-plan plan
write_shim sovereign-chat chat
write_shim sovereign-loop loop
write_shim sovereign-curiosity curiosity
write_shim sovereign-reason reason
write_shim sovereign-orchestrate orchestrate
write_shim sovereign-reflect reflect
echo "[shims] $BIN/sovereign-{plan,chat,loop,curiosity,reason,orchestrate,reflect}"
