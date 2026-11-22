#!/usr/bin/env python3
"""
Add capability-gate-check and gate purpose-run / execute lightly.
Uses TryCapabilityAndNodeCommands pattern if present; else inserts before final PrintUsage.
Avoids nested lambda damage.
"""
from pathlib import Path
import re
import sys

root = Path(sys.argv[1])
main = root / "apps/cli/main.cpp"
t = main.read_text()

if "capability-gate-check" in t and "CapabilityGate" in t:
    print("gate CLI already present")
    sys.exit(0)

if '#include "capability_gate.hpp"' not in t:
    t = '#include "capability_gate.hpp"\n' + t

# Usage lines — insert before Env line only if not present
if "capability-gate-check" not in t:
    anchor = '<< "Env: SOVEREIGN_DATA_ROOT'
    line = '        << "  sovereign-cli capability-gate-check <action-or-id>\\n"\n'
    if anchor in t:
        t = t.replace(anchor, line + "        " + anchor, 1)

# Extend TryCapabilityAndNodeCommands if it exists
helper_snip = r'''
    if (cmd == "capability-gate-check") {
        if (argc < 3) { std::cerr << "usage: capability-gate-check <action-or-id>\n"; return 2; }
        sovereign::capability::CapabilityRegistry caps;
        if (caps.Open(host.DataRoot()) != sovereign::capability::CapabilityError::Ok) {
            std::cerr << "capability open failed\n"; return 1;
        }
        (void)caps.SeedDefaults();
        sovereign::capability::CapabilityGate gate(caps);
        std::string key = argv[2];
        auto d = (key.find('.') != std::string::npos && key.rfind("host.", 0) == 0)
            ? gate.Require(key)
            : gate.RequireForAction(key);
        // also allow raw id without host. prefix
        if (!d.allowed && key.find('.') != std::string::npos)
            d = gate.Require(key);
        std::cout << "capability_id=" << d.capability_id
                  << " allowed=" << (d.allowed ? 1 : 0)
                  << " reason=" << d.reason << "\n";
        return d.allowed ? 0 : 3;
    }
'''

if "TryCapabilityAndNodeCommands" in t:
    # insert before final `return -1` or before node-events end return 2
    marker = "std::cerr << \"unknown node command\""
    if marker in t:
        t = t.replace(marker, helper_snip + "\n    " + marker, 1)
    else:
        # insert at start of function after opening brace of Try...
        m = re.search(r'int TryCapabilityAndNodeCommands\([^)]*\)\s*\{', t)
        if m:
            pos = m.end()
            t = t[:pos] + "\n" + helper_snip + t[pos:]
else:
    # standalone block before last PrintUsage in main
    block = f'''
    if (cmd == "capability-gate-check")
    {{
        if (argc < 3) {{ std::cerr << "usage: capability-gate-check <action-or-id>\\n"; return 2; }}
        sovereign::capability::CapabilityRegistry caps;
        if (caps.Open(host.DataRoot()) != sovereign::capability::CapabilityError::Ok)
        {{ std::cerr << "capability open failed\\n"; return 1; }}
        (void)caps.SeedDefaults();
        sovereign::capability::CapabilityGate gate(caps);
        std::string key = argv[2];
        auto d = gate.RequireForAction(key);
        if (!d.allowed) d = gate.Require(key);
        std::cout << "capability_id=" << d.capability_id
                  << " allowed=" << (d.allowed ? 1 : 0)
                  << " reason=" << d.reason << "\\n";
        return d.allowed ? 0 : 3;
    }}
'''
    idx = t.rfind("PrintUsage();")
    if idx < 0:
        print("FATAL no PrintUsage", file=sys.stderr)
        sys.exit(1)
    t = t[:idx] + block + "\n    " + t[idx:]

# L3.2–L3.5: gate purpose-run and execute at the start of those handlers
def inject_gate(cmd_pat: str, action: str, text: str) -> str:
    """After `if (cmd == \"purpose-run\"` opening brace, require gate."""
    # Find if (cmd == "purpose-run"
    patterns = [
        f'if (cmd == "{cmd_pat}"',
        f"if (cmd == '{cmd_pat}'",
    ]
    for pat in patterns:
        i = text.find(pat)
        if i < 0:
            continue
        brace = text.find("{", i)
        if brace < 0:
            continue
        # Don't double-inject
        window = text[brace : brace + 400]
        if "CapabilityGate" in window or "host.purpose" in window or "host.execute" in window:
            return text
        inject = f'''
        {{
            sovereign::capability::CapabilityRegistry _caps;
            if (_caps.Open(host.DataRoot()) == sovereign::capability::CapabilityError::Ok) {{
                (void)_caps.SeedDefaults();
                sovereign::capability::CapabilityGate _gate(_caps);
                auto _d = _gate.RequireForAction("{action}");
                if (!_d.allowed) {{
                    std::cerr << "capability_denied id=" << _d.capability_id
                              << " reason=" << _d.reason << "\\n";
                    host.Close();
                    return 3;
                }}
            }}
        }}
'''
        text = text[: brace + 1] + inject + text[brace + 1 :]
        print(f"gated {cmd_pat} -> {action}")
        return text
    print(f"skip gate inject for {cmd_pat}")
    return text

t = inject_gate("purpose-run", "purpose-run", t)
t = inject_gate("execute", "execute", t)

main.write_text(t)
print("CLI gate patch ok")
