#include "governance_service.hpp"
#include "memory_engine.hpp"
#include "orchestration_engine.hpp"
#include "test_require.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    using namespace sovereign;
    using namespace sovereign::cognitive;
    namespace gov = sovereign::governance;

    const auto gov_dir = std::filesystem::temp_directory_path() / "sov_orchestration_engine_gov";
    const auto mem_dir = std::filesystem::temp_directory_path() / "sov_orchestration_engine_mem";
    std::error_code ec;
    std::filesystem::remove_all(gov_dir, ec);
    std::filesystem::remove_all(mem_dir, ec);

    gov::GovernanceService governance;
    REQUIRE(governance.Open(gov_dir) == gov::GovernanceError::Ok);

    memory::MemoryEngine memory;
    REQUIRE(memory.Open(mem_dir) == memory::MemoryError::Ok);

    OrchestrationEngine orchestrator(&governance, &memory);
    PrincipalId requester{};
    requester.bytes[0] = 13;

    ExecutionPlan plan;
    plan.id = AnalysisId{};
    plan.requester = requester;
    plan.goal = "Ship canary build after approval";
    plan.created_unix_seconds = 1;
    PlanStep step1{"inspect", "Check state", false};
    PlanStep step2{"recommend", "Request approval", true};
    PlanStep step3{"execute", "Run approved action", true};
    plan.steps = {step1, step2, step3};
    plan.confidence = 0.9;
    plan.uncertainty_notes = "Governance-gated";

    auto seq = orchestrator.BuildSequence(plan);
    REQUIRE(seq.Ok());
    REQUIRE(seq.value.steps.size() == 3);
    REQUIRE(seq.value.steps[0].status == ExecutionStatus::Queued);
    REQUIRE(seq.value.steps[1].requires_governance);
    REQUIRE(seq.value.steps[2].requires_governance);

    gov::PrincipalId owner{};
    owner.bytes[0] = 1;
    gov::DecisionId did{};
    did.bytes = seq.value.decision_id;
    REQUIRE(governance.Approve(owner, true, did, "owner ok").Ok());

    auto exec = orchestrator.ExecuteApprovedPlan(seq.value, did);
    REQUIRE(exec.Ok());
    REQUIRE(exec.value.status == ExecutionStatus::Completed);

    governance.Close();
    memory.Close();
    std::filesystem::remove_all(gov_dir, ec);
    std::filesystem::remove_all(mem_dir, ec);
    std::cout << "cognitive_orchestration_engine_smoke_test: OK\n";
    return 0;
}
