#!/usr/bin/env python3
"""Inject capability_client require into sovereign_computer_use runtime.dispatch."""
from pathlib import Path
import re
import sys
import os

# Find installed CU package
candidates = [
    Path.home() / ".local/share/sovereign-computer-use/venv/lib",
    Path("/tmp/sov-cu/src"),
]
runtime = None
for base in candidates:
    if not base.exists():
        continue
    for p in base.rglob("runtime.py"):
        if "sovereign_computer_use" in str(p):
            runtime = p
            break
    if runtime:
        break

# Also check PYTHONPATH / pip show
if runtime is None:
    try:
        import sovereign_computer_use
        runtime = Path(sovereign_computer_use.__file__).parent / "runtime.py"
    except Exception:
        pass

if runtime is None or not runtime.exists():
    print("WARN: runtime.py not found — copy capability_client only")
    sys.exit(0)

# Install capability_client next to package
pkg = runtime.parent
client_src = Path(sys.argv[1]) if len(sys.argv) > 1 else None
# argv1 = path to capability_client.py content file
if len(sys.argv) > 1 and Path(sys.argv[1]).is_file():
    (pkg / "capability_client.py").write_text(Path(sys.argv[1]).read_text())
    print("wrote", pkg / "capability_client.py")

t = runtime.read_text()
if "capability_client" in t and "require_for_cu_action" in t:
    print("CU already gated")
    sys.exit(0)

# Inject at start of dispatch() after docstring/signature
m = re.search(r'def dispatch\([^)]*\):\s*\n', t)
if not m:
    print("no dispatch()", file=sys.stderr)
    sys.exit(1)

inject = '''
    # L4 capability gate (fail closed)
    try:
        from .capability_client import require_for_cu_action
        _gate = require_for_cu_action(action)
        if not _gate.get("ok", False):
            return _gate
    except Exception as _ex:
        return {"ok": False, "error": f"capability_gate_error: {_ex}", "denied": True}

'''
# Only gate non-ping tools
inject = '''
    # L4 capability gate (fail closed) — skip health/ping
    if action not in ("ping", "list_tools"):
        try:
            from .capability_client import require_for_cu_action
            _gate = require_for_cu_action(action)
            if not _gate.get("ok", False):
                return _gate
        except Exception as _ex:
            return {"ok": False, "error": f"capability_gate_error: {_ex}", "denied": True}

'''
pos = m.end()
t = t[:pos] + inject + t[pos:]
runtime.write_text(t)
print("patched", runtime)
