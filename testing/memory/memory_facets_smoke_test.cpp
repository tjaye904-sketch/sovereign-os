#include "memory_engine.hpp"
#include "memory_facets.hpp"

#include <filesystem>
#include <iostream>

#define REQUIRE(cond) \
    do { if (!(cond)) { std::cerr << "REQUIRE failed: " #cond "\n"; return 1; } } while (0)

int main()
{
    namespace fs = std::filesystem;
    auto dir = fs::temp_directory_path() / "sov-mem-facets";
    fs::remove_all(dir);
    fs::create_directories(dir);

    sovereign::memory::MemoryEngine eng;
    REQUIRE(eng.Open(dir) == sovereign::memory::MemoryError::Ok);

    sovereign::memory::MemoryRecord a;
    a.tags = {sovereign::memory::FacetTag(sovereign::memory::kFacetConstitutional), "arch"};
    a.body = "Sovereign is federation authority";
    REQUIRE(eng.Append(a).Ok());

    sovereign::memory::MemoryRecord b;
    b.tags = {sovereign::memory::FacetTag(sovereign::memory::kFacetWorking), "task"};
    b.body = "active leap L1.2";
    REQUIRE(eng.Append(b).Ok());

    auto cons = eng.ListByFacet(sovereign::memory::kFacetConstitutional);
    REQUIRE(cons.Ok());
    REQUIRE(cons.value.size() == 1);
    REQUIRE(cons.value[0].body.find("federation") != std::string::npos);

    auto counts = eng.CountByFacet();
    REQUIRE(counts.Ok());
    bool saw_c = false, saw_w = false;
    for (const auto& c : counts.value)
    {
        if (c.facet == sovereign::memory::kFacetConstitutional && c.count == 1)
            saw_c = true;
        if (c.facet == sovereign::memory::kFacetWorking && c.count == 1)
            saw_w = true;
    }
    REQUIRE(saw_c && saw_w);

    auto bad = eng.ListByFacet("not-a-facet");
    REQUIRE(!bad.Ok());

    eng.Close();
    fs::remove_all(dir);
    std::cout << "memory_facets_smoke_test: OK\n";
    return 0;
}
