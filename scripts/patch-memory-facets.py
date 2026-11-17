#!/usr/bin/env python3
"""L1.2: add ListByFacet + CountByFacet to MemoryEngine; include facets header."""
from pathlib import Path
import sys

root = Path(sys.argv[1])
hpp = root / "core/memory/memory_engine.hpp"
cpp = root / "core/memory/memory_engine.cpp"
fac = root / "core/memory/memory_facets.hpp"

if "ListByFacet" in hpp.read_text():
    print("memory engine already has ListByFacet")
else:
    t = hpp.read_text()
    if '#include "memory_facets.hpp"' not in t:
        t = t.replace('#include "memory_types.hpp"',
                      '#include "memory_types.hpp"\n#include "memory_facets.hpp"')
    decl = """
    // L1.2 — memory facets (constitutional / working / decision / …)
    Result<std::vector<MemoryRecord>> ListByFacet(const std::string& facet) const;
    Result<std::vector<FacetCount>> CountByFacet() const;
"""
    t = t.replace(
        "Result<std::vector<MemoryRecord>> ListByTag(const std::string& tag) const;",
        "Result<std::vector<MemoryRecord>> ListByTag(const std::string& tag) const;" + decl,
    )
    hpp.write_text(t)
    print("patched memory_engine.hpp")

t = cpp.read_text()
if "ListByFacet" in t:
    print("memory_engine.cpp already has ListByFacet")
else:
    methods = r'''
Result<std::vector<MemoryRecord>> MemoryEngine::ListByFacet(const std::string& facet) const
{
    if (!IsKnownFacet(facet))
    {
        Result<std::vector<MemoryRecord>> r;
        r.error = MemoryError::InvalidArgument;
        return r;
    }
    return ListByTag(FacetTag(facet));
}

Result<std::vector<FacetCount>> MemoryEngine::CountByFacet() const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<FacetCount>> r;
    if (!open_)
    {
        r.error = MemoryError::StorageFailure;
        return r;
    }
    for (const char* f : kAllFacets)
    {
        FacetCount c;
        c.facet = f;
        const std::string tag = FacetTag(f);
        for (const auto& rec : records_)
        {
            for (const auto& t : rec.tags)
            {
                if (t == tag)
                {
                    ++c.count;
                    break;
                }
            }
        }
        r.value.push_back(std::move(c));
    }
    r.error = MemoryError::Ok;
    return r;
}

'''
    t = t.replace("} // namespace sovereign::memory", methods + "} // namespace sovereign::memory")
    cpp.write_text(t)
    print("patched memory_engine.cpp")

# CMake - facets is header only, no change required
print("ok")
