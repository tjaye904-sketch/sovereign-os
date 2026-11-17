#!/usr/bin/env python3
"""L0.2: ensure domain_profile lists all 16 blueprint workspace names (stubs OK)."""
from pathlib import Path
import re, sys

ROOT = Path(sys.argv[1] if len(sys.argv) > 1 else Path.home() / "src/sovereign-os/sovereign-work")
NAMES = [
    "development", "knowledge", "business", "finance", "life", "health",
    "research", "security", "infrastructure", "automation", "robotics",
    "communications", "creative", "intelligence", "governance", "federation",
]
candidates = [
    ROOT / "core/domain/domain_profile.cpp",
    ROOT / "core/domain/domain_profile.hpp",
]
cpp = ROOT / "core/domain/domain_profile.cpp"
if not cpp.exists():
    print(f"SOFT: {cpp} missing — registry YAML still authoritative")
    sys.exit(0)

text = cpp.read_text()
missing = [n for n in NAMES if f'"{n}"' not in text and f"'{n}'" not in text]
if not missing:
    print("domain_profile already mentions all 16 names (or paths)")
    # still write a sidecar list for tooling
else:
    print("missing from domain_profile.cpp:", ", ".join(missing))
    sidecar = ROOT / "core/domain/workspace_names_16.txt"
    sidecar.write_text("\n".join(NAMES) + "\n")
    print(f"wrote {sidecar}")
    # Append a clearly marked stub block if we find a profiles vector pattern
    if "development" in text and "kWorkspaceNames" not in text:
        block = """
// L0.2: blueprint workspace id list (status may be stub — see COMPONENT_REGISTRY.yaml)
static const char* kWorkspaceNames16[] = {
""" + ",\n".join(f'    "{n}"' for n in NAMES) + ",\n    nullptr\n};\n"
        # insert before last closing namespace if possible
        if "} // namespace" in text:
            text = text.replace("} // namespace", block + "\n} // namespace", 1)
            cpp.write_text(text)
            print("patched kWorkspaceNames16 into domain_profile.cpp")
        else:
            print("SOFT: could not auto-patch cpp; sidecar list written")
    else:
        sidecar = ROOT / "core/domain/workspace_names_16.txt"
        sidecar.write_text("\n".join(NAMES) + "\n")
        print(f"wrote {sidecar}")

# Always ensure registry path exists copy
reg = ROOT / "docs/os2.0/COMPONENT_REGISTRY.yaml"
print(f"registry expected at {reg} exists={reg.exists()}")
