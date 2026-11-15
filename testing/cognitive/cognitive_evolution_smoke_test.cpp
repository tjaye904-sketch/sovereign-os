#include "evolution_engine.hpp"
#include "test_require.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    using namespace sovereign::cognitive;

    const auto repo_root = std::filesystem::path("/home/tjaye904/Documents/GitHub/sovereign-os");
    EvolutionEngine evolution(repo_root, nullptr);
    auto scan = evolution.ScanSystem(repo_root);
    REQUIRE(!scan.empty());
    REQUIRE(scan.front().subsystem.empty() == false);
    REQUIRE(scan.front().summary.empty() == false);

    auto backlog = evolution.NextEvolutionBatch(5);
    REQUIRE(!backlog.empty());
    REQUIRE(backlog.front().subsystem.empty() == false);

    std::cout << "cognitive_evolution_smoke_test: OK\n";
    return 0;
}
