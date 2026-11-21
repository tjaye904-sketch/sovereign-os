#!/usr/bin/env python3
from pathlib import Path
import sys
root = Path(sys.argv[1])
main = root / "apps/cli/main.cpp"
t = main.read_text()
if "node-register-local" in t:
    print("CLI node already")
    sys.exit(0)
if '#include "node_registry.hpp"' not in t:
    t = '#include "node_registry.hpp"\n' + t

usage = (
    '        << "  sovereign-cli node-register-local <name>\\n"\n'
    '        << "  sovereign-cli node-register-peer <name> <endpoint>\\n"\n'
    '        << "  sovereign-cli node-list\\n"\n'
    '        << "  sovereign-cli node-heartbeat <name>\\n"\n'
    '        << "  sovereign-cli node-bind <name> <workspace>\\n"\n'
    '        << "  sovereign-cli node-caps <name> <cap,cap,...>\\n"\n'
    '        << "  sovereign-cli node-events\\n"\n'
)
if '<< "Env: SOVEREIGN_DATA_ROOT' in t:
    t = t.replace('<< "Env: SOVEREIGN_DATA_ROOT', usage + '        << "Env: SOVEREIGN_DATA_ROOT', 1)

block = r'''
    if (cmd.rfind("node-", 0) == 0)
    {
        sovereign::node::NodeRegistry nodes;
        if (nodes.Open(host.DataRoot()) != sovereign::node::NodeError::Ok)
        {
            std::cerr << "node open failed\n";
            return 1;
        }
        if (cmd == "node-register-local")
        {
            if (argc < 3) { std::cerr << "usage: node-register-local <name>\n"; return 2; }
            auto r = nodes.RegisterLocal(argv[2]);
            if (!r.Ok()) { std::cerr << "register failed\n"; return 1; }
            std::cout << "node_id=" << sovereign::node::NodeRegistry::IdHex(r.value) << "\n";
            return 0;
        }
        if (cmd == "node-register-peer")
        {
            if (argc < 4) { std::cerr << "usage: node-register-peer <name> <endpoint>\n"; return 2; }
            auto r = nodes.RegisterPeer(argv[2], argv[3]);
            if (!r.Ok()) { std::cerr << "register failed\n"; return 1; }
            std::cout << "node_id=" << sovereign::node::NodeRegistry::IdHex(r.value) << "\n";
            return 0;
        }
        if (cmd == "node-list")
        {
            auto r = nodes.List();
            if (!r.Ok()) { std::cerr << "list failed\n"; return 1; }
            for (const auto& n : r.value)
            {
                std::cout << "name=" << n.name
                          << " kind=" << sovereign::node::KindName(n.kind)
                          << " health=" << sovereign::node::HealthName(n.health)
                          << " ws=" << n.workspace_name
                          << " caps=" << n.capabilities.size()
                          << "\n";
            }
            return 0;
        }
        if (cmd == "node-heartbeat")
        {
            if (argc < 3) { std::cerr << "usage: node-heartbeat <name>\n"; return 2; }
            auto f = nodes.FindByName(argv[2]);
            if (!f.Ok()) { std::cerr << "not found\n"; return 1; }
            auto r = nodes.Heartbeat(f.value.id);
            if (!r.Ok()) { std::cerr << "heartbeat failed\n"; return 1; }
            std::cout << "name=" << argv[2] << " health=healthy\n";
            return 0;
        }
        if (cmd == "node-bind")
        {
            if (argc < 4) { std::cerr << "usage: node-bind <name> <workspace>\n"; return 2; }
            auto f = nodes.FindByName(argv[2]);
            if (!f.Ok()) { std::cerr << "not found\n"; return 1; }
            auto r = nodes.BindWorkspace(f.value.id, argv[3]);
            if (!r.Ok()) { std::cerr << "bind failed\n"; return 1; }
            std::cout << "name=" << argv[2] << " workspace=" << argv[3] << "\n";
            return 0;
        }
        if (cmd == "node-caps")
        {
            if (argc < 4) { std::cerr << "usage: node-caps <name> <cap,cap,...>\n"; return 2; }
            auto f = nodes.FindByName(argv[2]);
            if (!f.Ok()) { std::cerr << "not found\n"; return 1; }
            std::vector<std::string> caps;
            std::string s = argv[3];
            std::string cur;
            for (char c : s) {
                if (c == ',') { if (!cur.empty()) caps.push_back(cur); cur.clear(); }
                else cur.push_back(c);
            }
            if (!cur.empty()) caps.push_back(cur);
            auto r = nodes.SetCapabilities(f.value.id, caps);
            if (!r.Ok()) { std::cerr << "caps failed\n"; return 1; }
            std::cout << "name=" << argv[2] << " caps=" << caps.size() << "\n";
            return 0;
        }
        if (cmd == "node-events")
        {
            auto r = nodes.RecentEvents(30);
            if (!r.Ok()) { std::cerr << "events failed\n"; return 1; }
            for (const auto& e : r.value) std::cout << e << "\n";
            return 0;
        }
        std::cerr << "unknown node command\n";
        return 2;
    }
'''
for m in ['if (cmd == "capability-seed")', 'if (cmd == "capability-list")', 'std::cerr << "unknown command"']:
    if m in t:
        t = t.replace(m, block + "\n    " + m, 1)
        break
else:
    print("FAIL cli insert", file=sys.stderr)
    sys.exit(1)
main.write_text(t)
print("CLI node ok")
