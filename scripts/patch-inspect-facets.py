#!/usr/bin/env python3
from pathlib import Path
import sys
root = Path(sys.argv[1])
p = root / "core/diagnostics/transparency_report.cpp"
if not p.exists():
    print("no transparency_report.cpp")
    sys.exit(0)
t = p.read_text()
if "facet_counts" in t or "CountByFacet" in t:
    print("inspect already has facets")
    sys.exit(0)
needle = 'o << "  provenance=append-only memory + semantic embed_model + audit\\n";'
insert = '''o << "  provenance=append-only memory + semantic embed_model + audit\\n";
        {
            auto fc = host_.Memory().CountByFacet();
            if (fc.Ok())
            {
                o << "  facet_counts";
                for (const auto& c : fc.value)
                    o << " " << c.facet << "=" << c.count;
                o << "\\n";
            }
        }
'''
if needle in t:
    t = t.replace(needle, insert, 1)
    # ensure include
    if "memory_facets.hpp" not in t and "memory_engine.hpp" in t:
        t = t.replace('#include "memory_engine.hpp"',
                      '#include "memory_engine.hpp"\n#include "memory_facets.hpp"')
    p.write_text(t)
    print("patched transparency_report.cpp")
else:
    print("SOFT: provenance line not found; skip inspect patch")
