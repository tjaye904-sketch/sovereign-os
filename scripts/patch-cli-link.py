#!/usr/bin/env python3
from pathlib import Path
import sys
root = Path(sys.argv[1])
cm = root / "apps/cli/CMakeLists.txt"
if not cm.exists():
    print("no apps/cli/CMakeLists.txt")
    sys.exit(0)
t = cm.read_text()
if "sovereign_secrets" not in t:
    # link against secrets
    if "target_link_libraries" in t:
        # append to first target_link_libraries block roughly
        t = t.replace(
            "target_link_libraries(",
            "target_link_libraries(\n",
            1,
        )
        # simpler: add line after sovereign_audit if present
        if "sovereign_audit" in t:
            t = t.replace("sovereign_audit", "sovereign_audit\n        sovereign_secrets", 1)
        else:
            t += "\ntarget_link_libraries(sovereign-cli PRIVATE sovereign_secrets)\n"
    else:
        t += "\ntarget_link_libraries(sovereign-cli PRIVATE sovereign_secrets)\n"
    cm.write_text(t)
    print("patched cli link")
else:
    print("cli already links secrets")
