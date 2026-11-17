#!/usr/bin/env python3
from pathlib import Path
import sys
root = Path(sys.argv[1])
cm = root / "CMakeLists.txt"
t = cm.read_text()
if "core/secrets" not in t:
    needle = "add_subdirectory(core/audit)"
    if needle in t:
        t = t.replace(needle, needle + "\nadd_subdirectory(core/secrets)", 1)
    else:
        t += "\nadd_subdirectory(core/secrets)\n"
    cm.write_text(t)
    print("patched root CMakeLists.txt")
else:
    print("root CMake already has secrets")

tc = root / "testing/CMakeLists.txt"
if tc.exists():
    tt = tc.read_text()
    line = "sovereign_add_smoke(secrets_manager_smoke_test secrets sovereign_secrets)\n"
    if "secrets_manager_smoke_test" not in tt:
        tt += "\n" + line
        tc.write_text(tt)
        print("patched testing/CMakeLists.txt")
    else:
        print("testing cmake already has secrets smoke")
