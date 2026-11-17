#!/usr/bin/env python3
from pathlib import Path
import sys
root = Path(sys.argv[1])
main = root / "apps/cli/main.cpp"
t = main.read_text()
if "memory-facet-add" in t:
    print("CLI already has memory-facet-add")
    sys.exit(0)

if '#include "memory_facets.hpp"' not in t:
    if '#include "memory_engine.hpp"' in t:
        t = t.replace('#include "memory_engine.hpp"',
                      '#include "memory_engine.hpp"\n#include "memory_facets.hpp"')
    else:
        t = '#include "memory_facets.hpp"\n' + t

usage = (
    '        << "  sovereign-cli memory-facet-add <facet> <tag> <body...>\\n"\n'
    '        << "  sovereign-cli memory-facet-list [facet]\\n"\n'
    '        << "  sovereign-cli memory-facet-counts\\n"\n'
)
if "memory-facet-add" not in t:
    if "sovereign-cli memory-rank" in t:
        t = t.replace(
            '<< "  sovereign-cli memory-rank\\n"',
            '<< "  sovereign-cli memory-rank\\n"\n' + usage,
            1,
        )
    elif "Env: SOVEREIGN_DATA_ROOT" in t:
        t = t.replace("Env: SOVEREIGN_DATA_ROOT", usage + "Env: SOVEREIGN_DATA_ROOT", 1)

block = r'''
    if (cmd == "memory-facet-add" || cmd == "memory-facet-list" || cmd == "memory-facet-counts")
    {
        if (cmd == "memory-facet-counts")
        {
            auto r = host.Memory().CountByFacet();
            if (!r.Ok())
            {
                std::cerr << "memory-facet-counts failed\n";
                return 1;
            }
            for (const auto& c : r.value)
                std::cout << "facet=" << c.facet << " count=" << c.count << "\n";
            return 0;
        }
        if (cmd == "memory-facet-list")
        {
            std::string facet = (argc >= 3) ? argv[2] : "";
            if (facet.empty())
            {
                auto all = host.Memory().ListAll();
                if (!all.Ok())
                {
                    std::cerr << "memory-facet-list failed\n";
                    return 1;
                }
                for (const auto& rec : all.value)
                {
                    auto f = sovereign::memory::FacetFromTags(rec.tags);
                    if (f.empty())
                        continue;
                    std::cout << "facet=" << f << " body=" << rec.body.substr(0, 120) << "\n";
                }
                return 0;
            }
            auto r = host.Memory().ListByFacet(facet);
            if (!r.Ok())
            {
                std::cerr << "memory-facet-list failed (unknown facet?)\n";
                return 1;
            }
            for (const auto& rec : r.value)
                std::cout << "body=" << rec.body << "\n";
            return 0;
        }
        if (cmd == "memory-facet-add")
        {
            if (argc < 5)
            {
                std::cerr << "usage: memory-facet-add <facet> <tag> <body...>\n";
                std::cerr << "facets: constitutional working decision operational knowledge execution\n";
                return 2;
            }
            std::string facet = argv[2];
            if (!sovereign::memory::IsKnownFacet(facet))
            {
                std::cerr << "unknown facet\n";
                return 2;
            }
            std::string tag = argv[3];
            std::string body;
            for (int i = 4; i < argc; ++i)
            {
                if (!body.empty())
                    body.push_back(' ');
                body += argv[i];
            }
            sovereign::memory::MemoryRecord rec;
            rec.tags.push_back(sovereign::memory::FacetTag(facet));
            rec.tags.push_back(tag);
            rec.body = body;
            auto r = host.Memory().Append(rec);
            if (!r.Ok())
            {
                std::cerr << "memory-facet-add failed\n";
                return 1;
            }
            std::cout << "facet=" << facet << " record stored\n";
            return 0;
        }
    }
'''

if 'cmd == "memory-facet-add"' not in t:
    for m in ['if (cmd == "memory-rank")', 'if (cmd == "memory-add")', 'std::cerr << "unknown command"']:
        if m in t:
            t = t.replace(m, block + "\n    " + m, 1)
            break
    else:
        print("FAIL insert CLI", file=sys.stderr)
        sys.exit(1)

main.write_text(t)
print("patched CLI memory facets")
