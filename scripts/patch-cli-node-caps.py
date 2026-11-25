#!/usr/bin/env python3
"""Add node-caps-bind/list/unbind to apps/cli/main.cpp if capability-list exists."""
from pathlib import Path
import sys

root = Path(sys.argv[1])
main = root / "apps" / "cli" / "main.cpp"
if not main.exists():
    print("no main.cpp"); sys.exit(0)
t = main.read_text()
if "node-caps-bind" in t:
    print("cli already has node-caps"); sys.exit(0)

usage = '  sovereign-cli node-caps-bind <node> <capability-id>\n'
if "capability-list" in t and usage.strip() not in t:
    t = t.replace(
        "capability-list",
        "capability-list\n  sovereign-cli node-caps-bind <node> <capability-id>\n"
        "  sovereign-cli node-caps-list <node>\n"
        "  sovereign-cli node-caps-unbind <node> <capability-id>",
        1,
    )

# include
if '#include' in t and "node_capability_binding.hpp" not in t:
    # after capability includes if any
    ins = '#include "node_capability_binding.hpp"\n'
    if "capability_registry.hpp" in t:
        t = t.replace('#include "capability_registry.hpp"',
                      '#include "capability_registry.hpp"\n#include "node_capability_binding.hpp"')
    else:
        t = ins + t

handler = r'''
  if (cmd == "node-caps-bind") {
    if (argc < 4) { std::cerr << "usage: node-caps-bind <node> <capability-id>\n"; return 2; }
    sovereign::capability::CapabilityRegistry reg(data_root);
    sovereign::capability::NodeCapabilityBinding bind(data_root);
    auto r = bind.Bind(argv[2], argv[3], reg, true);
    if (!r.ok) { std::cout << "error=" << r.error << "\n"; return 1; }
    std::cout << "bound node=" << argv[2] << " capability=" << argv[3] << "\n";
    return 0;
  }
  if (cmd == "node-caps-list") {
    if (argc < 3) { std::cerr << "usage: node-caps-list <node>\n"; return 2; }
    sovereign::capability::NodeCapabilityBinding bind(data_root);
    for (const auto& id : bind.List(argv[2])) std::cout << "capability=" << id << "\n";
    return 0;
  }
  if (cmd == "node-caps-unbind") {
    if (argc < 4) { std::cerr << "usage: node-caps-unbind <node> <capability-id>\n"; return 2; }
    sovereign::capability::NodeCapabilityBinding bind(data_root);
    auto r = bind.Unbind(argv[2], argv[3]);
    if (!r.ok) { std::cout << "error=" << r.error << "\n"; return 1; }
    std::cout << "unbound node=" << argv[2] << " capability=" << argv[3] << "\n";
    return 0;
  }
'''

# insert before a late return or unknown command
marker = 'if (cmd == "capability-list")'
if marker in t and "node-caps-bind" not in t.split("int main")[-1][:5000]:
    # append handlers near capability-list block — simple: before final usage print
    idx = t.rfind("return 1;")
    if idx > 0:
        t = t[:idx] + handler + "\n  " + t[idx:]

main.write_text(t)
print("patched cli main.cpp (review if compile fails)")
