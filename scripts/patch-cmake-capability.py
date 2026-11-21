#!/usr/bin/env python3
from pathlib import Path
import sys
root = Path(sys.argv[1])
cm = root / "CMakeLists.txt"
t = cm.read_text()
if "core/capability" not in t:
    for needle in ["add_subdirectory(core/secrets)", "add_subdirectory(core/audit)", "add_subdirectory(core/workspace)"]:
        if needle in t:
            t = t.replace(needle, needle + "\nadd_subdirectory(core/capability)", 1)
            break
    else:
        t += "\nadd_subdirectory(core/capability)\n"
    cm.write_text(t)
    print("root cmake: capability")
tc = root / "testing/CMakeLists.txt"
if tc.exists() and "capability_registry_smoke_test" not in tc.read_text():
    tc.write_text(tc.read_text() + "\nsovereign_add_smoke(capability_registry_smoke_test capability sovereign_capability)\n")
    print("testing cmake: smoke")
cli = root / "apps/cli/CMakeLists.txt"
if cli.exists():
    ct = cli.read_text()
    if "sovereign_capability" not in ct:
        if "sovereign_secrets" in ct:
            ct = ct.replace("sovereign_secrets", "sovereign_secrets\n        sovereign_capability", 1)
        elif "sovereign_audit" in ct:
            ct = ct.replace("sovereign_audit", "sovereign_audit\n        sovereign_capability", 1)
        else:
            ct += "\ntarget_link_libraries(sovereign-cli PRIVATE sovereign_capability)\n"
        cli.write_text(ct)
        print("cli linked")
