#!/usr/bin/env python3
from pathlib import Path
import sys
root = Path(sys.argv[1])
main = root / "apps/cli/main.cpp"
t = main.read_text()
if "capability-list" in t:
    print("CLI already")
    sys.exit(0)
if '#include "capability_registry.hpp"' not in t:
    t = '#include "capability_registry.hpp"\n' + t

usage = (
    '        << "  sovereign-cli capability-seed\\n"\n'
    '        << "  sovereign-cli capability-list\\n"\n'
    '        << "  sovereign-cli capability-enable <id>\\n"\n'
    '        << "  sovereign-cli capability-disable <id>\\n"\n'
    '        << "  sovereign-cli capability-check <id>\\n"\n'
)
if "Env: SOVEREIGN_DATA_ROOT" in t:
    t = t.replace("Env: SOVEREIGN_DATA_ROOT", usage + "Env: SOVEREIGN_DATA_ROOT", 1)

block = r'''
    if (cmd == "capability-seed" || cmd == "capability-list" || cmd == "capability-enable"
        || cmd == "capability-disable" || cmd == "capability-check")
    {
        sovereign::capability::CapabilityRegistry caps;
        if (caps.Open(host.DataRoot()) != sovereign::capability::CapabilityError::Ok)
        {
            std::cerr << "capability open failed\n";
            return 1;
        }
        if (cmd == "capability-seed")
        {
            auto r = caps.SeedDefaults();
            if (!r.Ok()) { std::cerr << "seed failed\n"; return 1; }
            std::cout << "seeded_added=" << r.value << "\n";
            return 0;
        }
        if (cmd == "capability-list")
        {
            auto r = caps.List();
            if (!r.Ok()) { std::cerr << "list failed\n"; return 1; }
            if (r.value.empty())
            {
                auto s = caps.SeedDefaults();
                (void)s;
                r = caps.List();
            }
            for (const auto& c : r.value)
            {
                std::cout << "id=" << c.id
                          << " enabled=" << (c.enabled ? 1 : 0)
                          << " source=" << c.source
                          << " desc=" << c.description << "\n";
            }
            return 0;
        }
        if (argc < 3) {
            std::cerr << "usage: " << cmd << " <id>\n";
            return 2;
        }
        std::string id = argv[2];
        if (cmd == "capability-enable" || cmd == "capability-disable")
        {
            auto r = caps.SetEnabled(id, cmd == "capability-enable");
            if (!r.Ok()) { std::cerr << "set failed\n"; return 1; }
            std::cout << "id=" << id << " enabled=" << (cmd == "capability-enable" ? 1 : 0) << "\n";
            return 0;
        }
        auto r = caps.IsEnabled(id);
        if (!r.Ok()) { std::cerr << "check failed\n"; return 1; }
        std::cout << "id=" << id << " enabled=" << (r.value ? 1 : 0) << "\n";
        return 0;
    }
'''
for m in ['if (cmd == "workspace-status")', 'if (cmd == "secret-put")', 'std::cerr << "unknown command"']:
    if m in t:
        t = t.replace(m, block + "\n    " + m, 1)
        break
else:
    print("FAIL insert", file=sys.stderr)
    sys.exit(1)
main.write_text(t)
print("CLI capability commands")
