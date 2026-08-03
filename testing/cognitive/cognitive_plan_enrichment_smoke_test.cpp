#include "cognitive_service.hpp"
#include "governance_service.hpp"
#include "test_require.hpp"
#include <filesystem>
#include <iostream>
int main()
{
    using namespace sovereign::cognitive;
    namespace gov = sovereign::governance;
    const auto dir = std::filesystem::temp_directory_path() / "sov_plan_enrich";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    gov::GovernanceService governance;
    REQUIRE(governance.Open(dir) == gov::GovernanceError::Ok);
    CognitiveService cog(governance, nullptr, nullptr, "openclaw");
    sovereign::PrincipalId actor{};
    const std::string goal = "Test goal for heuristic plan";
    auto plan = cog.BuildPlan(actor, goal);
    REQUIRE(plan.Ok());
    REQUIRE(plan.value.steps.size() >= 3);
    REQUIRE(plan.value.enrichment_status == "no_router" ||
            plan.value.enrichment_status == "heuristic");
    REQUIRE(plan.value.goal == goal);
    REQUIRE(plan.value.steps[1].detail.find(goal) != std::string::npos);
    std::cout << "cognitive_plan_enrichment_smoke_test: OK\n";
    return 0;
}
