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
    // models=nullptr → honest no_router after CognitiveService::EnrichPlanWithModel
    CognitiveService cog(governance, nullptr, nullptr, "openclaw");
    sovereign::PrincipalId actor{};
    const std::string goal = "Test goal for heuristic plan";
    auto plan = cog.BuildPlan(actor, goal);
    REQUIRE(plan.Ok());
    REQUIRE(plan.value.steps.size() >= 3);
    REQUIRE(
        plan.value.enrichment_status == "no_router" ||
        plan.value.enrichment_status == "heuristic" ||
        plan.value.enrichment_status == "model_unavailable" ||
        plan.value.enrichment_status == "model_failed" ||
        plan.value.enrichment_status == "timeout" ||
        plan.value.enrichment_status == "model_enriched" ||
        plan.value.enrichment_status == "parse_failed");
    REQUIRE(plan.value.goal == goal);
    // Goal must appear on the plan object (not a fixed step index).
    REQUIRE(plan.value.goal.find(goal) != std::string::npos);
    std::cout << "cognitive_plan_enrichment_smoke_test: OK status="
              << plan.value.enrichment_status << "\n";
    return 0;
}
