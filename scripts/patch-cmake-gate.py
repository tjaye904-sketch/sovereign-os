#!/usr/bin/env python3
from pathlib import Path
import sys
root = Path(sys.argv[1])
cm = root / "core/capability/CMakeLists.txt"
if not cm.exists():
    print("no capability CMakeLists", file=sys.stderr)
    sys.exit(1)
t = cm.read_text()
if "capability_gate.cpp" not in t:
    # add source to library
    if "capability_registry.cpp" in t:
        t = t.replace("capability_registry.cpp", "capability_registry.cpp capability_gate.cpp")
    else:
        t += "\ntarget_sources(sovereign_capability PRIVATE capability_gate.cpp)\n"
    cm.write_text(t)
    print("cmake gate source ok")
else:
    print("already")
tc = root / "testing/CMakeLists.txt"
if tc.exists() and "capability_gate_smoke_test" not in tc.read_text():
    tc.write_text(
        tc.read_text()
        + "\nsovereign_add_smoke(capability_gate_smoke_test capability sovereign_capability)\n"
    )
print("test wired")
