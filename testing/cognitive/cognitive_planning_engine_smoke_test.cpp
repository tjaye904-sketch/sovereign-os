#include "governance_service.hpp"
#include "memory_engine.hpp"
#include "planning_engine.hpp"
#include "test_require.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    using namespace sovereign;
    using namespace sovereign::cognitive;
    namespace gov = sovereign::governance;

    const auto gov_dir = std::filesystem::temp_directory_path() / "sov_planning_engine_gov";
    const auto mem_dir = std::filesystem::temp_directory_path() / "sov_planning_engine_mem";
    std::error_code ec;
    std::filesystem::remove_all(gov_dir, ec);
    std::filesystem::remove_all(mem_dir, ec);

    gov::GovernanceService governance;
    REQUIRE(governance.Open(gov_dir) == gov::GovernanceError::Ok);

    memory::MemoryEngine memory;
    REQUIRE(memory.Open(mem_dir) == memory::MemoryError::Ok);

    PlanningEngine planner(&governance, &memory);
    PrincipalId requester{};
    requester.bytes[0] = 11;

    auto plan = planner.BuildPlan(requester, "Ship a canary build after validation");
    REQUIRE(plan.Ok());
    REQUIRE(plan.value.goal == "Ship a canary build after validation");
    REQUIRE(plan.value.steps.size() >= 4);
    REQUIRE(plan.value.steps[0].action == "inspect");
    REQUIRE(plan.value.steps.back().requires_governance);
    REQUIRE(plan.value.confidence > 0.0);
    REQUIRE(!plan.value.enrichment_status.empty());

    governance.Close();
    memory.Close();
    std::filesystem::remove_all(gov_dir, ec);
    std::filesystem::remove_all(mem_dir, ec);
    std::cout << "cognitive_planning_engine_smoke_test: OK\n";
    return 0;
}
