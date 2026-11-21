#!/usr/bin/env python3
from pathlib import Path
import sys
root = Path(sys.argv[1])
cm = root / "CMakeLists.txt"
t = cm.read_text()
if "core/node" not in t:
    for n in ["add_subdirectory(core/capability)", "add_subdirectory(core/workspace)", "add_subdirectory(core/audit)"]:
        if n in t:
            t = t.replace(n, n + "\nadd_subdirectory(core/node)", 1)
            break
    else:
        t += "\nadd_subdirectory(core/node)\n"
    cm.write_text(t)
tc = root / "testing/CMakeLists.txt"
if tc.exists() and "node_registry_smoke_test" not in tc.read_text():
    tc.write_text(tc.read_text() + "\nsovereign_add_smoke(node_registry_smoke_test node sovereign_node)\n")
cli = root / "apps/cli/CMakeLists.txt"
if cli.exists() and "sovereign_node" not in cli.read_text():
    ct = cli.read_text()
    if "sovereign_capability" in ct:
        ct = ct.replace("sovereign_capability", "sovereign_capability\n        sovereign_node", 1)
    else:
        ct += "\ntarget_link_libraries(sovereign-cli PRIVATE sovereign_node)\n"
    cli.write_text(ct)
print("cmake node ok")
