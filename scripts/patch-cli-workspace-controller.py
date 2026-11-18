#!/usr/bin/env python3
from pathlib import Path
import sys
root = Path(sys.argv[1])
main = root / "apps/cli/main.cpp"
t = main.read_text()
if "workspace-suspend" in t:
    print("CLI already has workspace-suspend")
    sys.exit(0)

usage = (
    '        << "  sovereign-cli workspace-activate <name-or-hex>\\n"\n'
    '        << "  sovereign-cli workspace-suspend <name-or-hex>\\n"\n'
    '        << "  sovereign-cli workspace-status [name]\\n"\n'
)
if "workspace-list" in t:
    t = t.replace(
        '<< "  sovereign-cli workspace-list\\n"',
        '<< "  sovereign-cli workspace-list\\n"\n' + usage,
        1,
    )
elif "Env: SOVEREIGN_DATA_ROOT" in t:
    t = t.replace("Env: SOVEREIGN_DATA_ROOT", usage + "Env: SOVEREIGN_DATA_ROOT", 1)

# Helper block — resolve name or hex id
block = r'''
    if (cmd == "workspace-activate" || cmd == "workspace-suspend" || cmd == "workspace-status")
    {
        auto resolve = [&](const std::string& key) -> sovereign::workspace::Result<sovereign::workspace::Workspace> {
            sovereign::workspace::Result<sovereign::workspace::Workspace> r;
            if (key.size() == 32)
            {
                sovereign::workspace::WorkspaceId id{};
                // try hex via List scan
                auto all = host.Workspace().List();
                if (!all.Ok()) { r.error = all.error; return r; }
                for (const auto& w : all.value)
                {
                    bool match = true;
                    static const char* hx = "0123456789abcdef";
                    std::string h(32, '0');
                    for (size_t i = 0; i < 16; ++i) {
                        h[i*2] = hx[(w.id.bytes[i] >> 4) & 0xF];
                        h[i*2+1] = hx[w.id.bytes[i] & 0xF];
                    }
                    if (h == key) { r.value = w; return r; }
                }
            }
            return host.Workspace().FindByName(key);
        };
        if (cmd == "workspace-status")
        {
            if (argc >= 3)
            {
                auto r = resolve(argv[2]);
                if (!r.Ok()) { std::cerr << "workspace not found\n"; return 1; }
                std::cout << "name=" << r.value.name
                          << " kind=" << r.value.kind
                          << " status=" << sovereign::workspace::WorkspaceService::StatusName(r.value.status)
                          << "\n";
                return 0;
            }
            auto all = host.Workspace().List();
            if (!all.Ok()) { std::cerr << "list failed\n"; return 1; }
            for (const auto& w : all.value)
            {
                std::cout << "name=" << w.name
                          << " kind=" << w.kind
                          << " status=" << sovereign::workspace::WorkspaceService::StatusName(w.status)
                          << "\n";
            }
            return 0;
        }
        if (argc < 3) {
            std::cerr << "usage: " << cmd << " <name-or-hex>\n";
            return 2;
        }
        auto r = resolve(argv[2]);
        if (!r.Ok()) { std::cerr << "workspace not found\n"; return 1; }
        sovereign::workspace::Result<void> op;
        if (cmd == "workspace-activate")
            op = host.Workspace().Activate(r.value.id);
        else
            op = host.Workspace().Suspend(r.value.id);
        if (!op.Ok()) { std::cerr << cmd << " failed\n"; return 1; }
        auto again = host.Workspace().Get(r.value.id);
        std::cout << "name=" << r.value.name
                  << " status=" << sovereign::workspace::WorkspaceService::StatusName(again.value.status)
                  << "\n";
        return 0;
    }
'''

if 'cmd == "workspace-suspend"' not in t:
    for m in ['if (cmd == "workspace-list")', 'if (cmd == "workspace-create"', 'std::cerr << "unknown command"']:
        if m in t:
            t = t.replace(m, block + "\n    " + m, 1)
            break
    else:
        print("FAIL: no CLI insert point", file=sys.stderr)
        sys.exit(1)

main.write_text(t)
print("CLI patched")
