#!/usr/bin/env python3
"""Add secret-put/list/exists/delete/get CLI commands."""
from pathlib import Path
import sys
root = Path(sys.argv[1])
main = root / "apps/cli/main.cpp"
t = main.read_text()
if "secret-put" in t:
    print("CLI already has secret-put")
    sys.exit(0)

# include
if '#include "secret_manager.hpp"' not in t:
    # after audit include if present
    if '#include "audit_log.hpp"' in t:
        t = t.replace('#include "audit_log.hpp"',
                      '#include "audit_log.hpp"\n#include "secret_manager.hpp"', 1)
    else:
        t = '#include "secret_manager.hpp"\n' + t

usage = '        << "  sovereign-cli secret-put <name> <value>\\n"\n'
usage += '        << "  sovereign-cli secret-get <name>\\n"\n'
usage += '        << "  sovereign-cli secret-list\\n"\n'
usage += '        << "  sovereign-cli secret-exists <name>\\n"\n'
usage += '        << "  sovereign-cli secret-delete <name>\\n"\n'
if "secret-put" not in t and "sovereign-cli audit-verify" in t:
    t = t.replace(
        '<< "  sovereign-cli audit-verify\\n"',
        '<< "  sovereign-cli audit-verify\\n"\n' + usage,
        1,
    )
elif "secret-put" not in t:
    # generic: before Env:
    if 'Env: SOVEREIGN_DATA_ROOT' in t:
        t = t.replace(
            'Env: SOVEREIGN_DATA_ROOT',
            usage + 'Env: SOVEREIGN_DATA_ROOT',
            1,
        )

block = r'''
    if (cmd == "secret-put" || cmd == "secret-get" || cmd == "secret-list"
        || cmd == "secret-exists" || cmd == "secret-delete")
    {
        sovereign::secrets::SecretManager secrets;
        if (secrets.Open(host.DataRoot()) != sovereign::secrets::SecretError::Ok)
        {
            std::cerr << "secrets open failed\n";
            return 1;
        }
        secrets.SetAuditHook([&](const std::string& action, const std::string& name) {
            sovereign::audit::AuditEntry e;
            e.category = "secrets";
            e.action = action;
            e.subject = name;
            e.detail = "secret_manager";
            (void)host.Audit().Append(e);
        });
        if (cmd == "secret-put")
        {
            if (argc < 4)
            {
                std::cerr << "usage: secret-put <name> <value>\n";
                return 2;
            }
            auto r = secrets.Put(argv[2], argv[3]);
            if (!r.Ok())
            {
                std::cerr << "secret-put failed\n";
                return 1;
            }
            std::cout << "secret_stored name=" << argv[2] << "\n";
            return 0;
        }
        if (cmd == "secret-get")
        {
            if (argc < 3)
            {
                std::cerr << "usage: secret-get <name>\n";
                return 2;
            }
            auto r = secrets.Get(argv[2]);
            if (!r.Ok())
            {
                std::cerr << "secret-get failed\n";
                return 1;
            }
            std::cout << r.value << "\n";
            return 0;
        }
        if (cmd == "secret-list")
        {
            auto r = secrets.List();
            if (!r.Ok())
            {
                std::cerr << "secret-list failed\n";
                return 1;
            }
            for (const auto& m : r.value)
            {
                std::cout << "name=" << m.name << " bytes=" << m.value_bytes
                          << " updated=" << m.updated_unix << "\n";
            }
            return 0;
        }
        if (cmd == "secret-exists")
        {
            if (argc < 3)
            {
                std::cerr << "usage: secret-exists <name>\n";
                return 2;
            }
            auto r = secrets.Exists(argv[2]);
            if (!r.Ok())
            {
                std::cerr << "secret-exists failed\n";
                return 1;
            }
            std::cout << "exists=" << (r.value ? 1 : 0) << "\n";
            return 0;
        }
        if (cmd == "secret-delete")
        {
            if (argc < 3)
            {
                std::cerr << "usage: secret-delete <name>\n";
                return 2;
            }
            auto r = secrets.Delete(argv[2]);
            if (!r.Ok())
            {
                std::cerr << "secret-delete failed\n";
                return 1;
            }
            std::cout << "secret_deleted name=" << argv[2] << "\n";
            return 0;
        }
    }
'''

# Insert before a known late command or before final return of main unknown
if 'cmd == "secret-put"' not in t:
    markers = [
        'if (cmd == "audit-verify")',
        'if (cmd == "inspect")',
        'std::cerr << "unknown command"',
    ]
    inserted = False
    for m in markers:
        if m in t:
            t = t.replace(m, block + "\n    " + m, 1)
            inserted = True
            break
    if not inserted:
        print("WARN: could not find insert point for CLI block", file=sys.stderr)
        sys.exit(1)

main.write_text(t)
print("patched apps/cli/main.cpp")
