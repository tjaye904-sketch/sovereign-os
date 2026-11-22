#!/usr/bin/env python3
from pathlib import Path
import re
import sys
cpp = Path(sys.argv[1]) / "core/capability/capability_registry.cpp"
t = cpp.read_text()
extras = [
    ('host.execute', 'Governed execute'),
    ('host.purpose', 'Purpose automation'),
    ('host.computer_use', 'Computer use / browser'),
    ('host.sys', 'Sys app open'),
    ('host.voice', 'Voice speak/listen'),
]
if all(f'"{e[0]}"' in t for e in extras):
    print("seeds already present")
    sys.exit(0)

# Find SeedDefaults function and a push/register pattern
# Look for existing id strings like "host.cli"
m = re.search(r'(SeedDefaults\s*\([^)]*\)[^{]*\{)', t)
if not m:
    print("no SeedDefaults", file=sys.stderr)
    sys.exit(1)

# Try to find one example registration line to clone
# Patterns: {"id", "desc"} or Register("id"
clone = None
for line in t.splitlines():
    if "host.cli" in line or "model.chat" in line:
        clone = line
        break

insert_lines = []
for cid, desc in extras:
    if f'"{cid}"' in t:
        continue
    if clone and "host.cli" in clone:
        insert_lines.append(clone.replace("host.cli", cid).replace("Host CLI", desc).replace("host CLI", desc))
    else:
        # generic - many registries use structured init
        insert_lines.append(
            f'    // L3 seed\n'
            f'    if (IsEnabled("{cid}").error != CapabilityError::Ok) {{\n'
            f'      CapabilityRecord rec;\n'
            f'      rec.id = "{cid}";\n'
            f'      rec.description = "{desc}";\n'
            f'      rec.enabled = true;\n'
            f'      rec.source = "seed";\n'
            f'      // store via existing SetEnabled after inventing record — fallback Seed path\n'
            f'    }}\n'
        )

# Simpler reliable approach: after SeedDefaults returns, we can't.
# Append method SeedHostCaps to cpp
append = r'''

namespace sovereign::capability {

Result<int> CapabilityRegistry::SeedHostAutomationCaps()
{
    // Implemented as SeedDefaults + enable known host.* ids if present after seed.
    Result<int> r = SeedDefaults();
    int n = r.Ok() ? static_cast<int>(r.value) : 0;
    const char* ids[] = {
        "host.execute", "host.purpose", "host.computer_use", "host.sys", "host.voice",
        "host.cli", "model.chat", "memory.write"
    };
    for (const char* id : ids) {
        auto e = SetEnabled(id, true);
        if (e.Ok()) ++n;
    }
    Result<int> out;
    out.value = n;
    return out;
}

} // namespace sovereign::capability
'''

# Actually SeedDefaults must create records. Read file for how records are added.
print("--- SeedDefaults excerpt ---")
start = t.find("SeedDefaults")
print(t[start:start+1200])

# Write analysis for INSTALL to handle
Path("/tmp/seed_excerpt.txt").write_text(t[start:start+2000] if start>=0 else "missing")

if "SeedHostAutomationCaps" not in t and False:
    t += append
    cpp.write_text(t)

print("review excerpt written; INSTALL will parse")
sys.exit(0)
