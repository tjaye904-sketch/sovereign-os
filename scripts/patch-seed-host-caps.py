#!/usr/bin/env python3
"""Ensure SeedDefaults registers host.* capabilities for L3 gates."""
from pathlib import Path
import sys
root = Path(sys.argv[1])
cpp = root / "core/capability/capability_registry.cpp"
if not cpp.exists():
    print("no capability_registry.cpp", file=sys.stderr)
    sys.exit(1)
t = cpp.read_text()
needed = [
    ("host.execute", "Governed action execute"),
    ("host.purpose", "Purpose-driven host automation"),
    ("host.computer_use", "Browser and computer-use server"),
    ("host.sys", "System app open and desktop sys"),
    ("host.voice", "Speak and listen voice path"),
]
changed = False
# Find SeedDefaults body — add entries if missing
for cid, desc in needed:
    if f'"{cid}"' in t or f"'{cid}'" in t:
        continue
    # Try insert before return of SeedDefaults
    needle = "SeedDefaults"
    if needle not in t:
        print("SeedDefaults not found", file=sys.stderr)
        sys.exit(1)
    # Append via Register-like pattern used in file
    insert = None
    if "Register(" in t or "Upsert(" in t or "Add(" in t:
        # common pattern: records.push_back or RegisterDefault
        for fn in ["RegisterDefault", "Upsert", "Register", "AddCapability"]:
            if fn + "(" in t:
                insert = f'    {fn}("{cid}", "{desc}", true);\n'
                break
    if insert is None:
        # push_back CapabilityRecord style
        insert = (
            f'    {{ CapabilityRecord r; r.id = "{cid}"; r.description = "{desc}"; '
            f'r.enabled = true; r.source = "seed"; records_.push_back(r); }}\n'
        )
    # Insert before "return" inside SeedDefaults — heuristic: after first seed block
    if "host.execute" not in t:
        # place before closing of SeedDefaults function — after other seeds
        idx = t.find("SeedDefaults")
        # find next return after SeedDefaults
        ret = t.find("return", idx)
        if ret > 0:
            t = t[:ret] + insert + t[ret:]
            changed = True
            print("seeded", cid)
        else:
            t += "\n// L3 seed fallback\n" + insert
            changed = True
    else:
        print("already", cid)

# Better approach: rewrite SeedDefaults list if we find a clear vector of pairs
if "host.computer_use" not in t:
    # force append function at end of namespace
    extra = r'''
// L3 host capability seeds (idempotent helper)
namespace sovereign::capability {
void SeedHostAutomationCapabilities(CapabilityRegistry& reg) {
  struct E { const char* id; const char* desc; };
  const E items[] = {
    {"host.execute", "Governed action execute"},
    {"host.purpose", "Purpose-driven host automation"},
    {"host.computer_use", "Browser and computer-use"},
    {"host.sys", "System app open / desktop"},
    {"host.voice", "Speak and listen"},
  };
  for (const auto& e : items) {
    auto en = reg.IsEnabled(e.id);
    if (!en.Ok()) {
      // try enable path via SetEnabled after ensuring exists
      (void)reg.SetEnabled(e.id, true); // may fail if not registered
    }
  }
  (void)reg;
}
} // namespace
'''
    # Instead patch SeedDefaults to call Register for each — read actual SeedDefaults
    pass

cpp.write_text(t)
print("patch-seed done changed=", changed)
