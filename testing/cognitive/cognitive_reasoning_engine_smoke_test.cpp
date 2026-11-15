#include "decision_engine.hpp"
#include "governance_service.hpp"
#include "memory_engine.hpp"
#include "reasoning_engine.hpp"
#include "reflection_engine.hpp"
#include "test_require.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    using namespace sovereign;
    using namespace sovereign::cognitive;
    namespace gov = sovereign::governance;

    const auto gov_dir = std::filesystem::temp_directory_path() / "sov_reasoning_engine_gov";
    const auto mem_dir = std::filesystem::temp_directory_path() / "sov_reasoning_engine_mem";
    std::error_code ec;
    std::filesystem::remove_all(gov_dir, ec);
    std::filesystem::remove_all(mem_dir, ec);

    gov::GovernanceService governance;
    REQUIRE(governance.Open(gov_dir) == gov::GovernanceError::Ok);

    memory::MemoryEngine memory;
    REQUIRE(memory.Open(mem_dir) == memory::MemoryError::Ok);

    ReasoningEngine reasoning(&memory);
    PrincipalId user{};
    user.bytes[0] = 7;

    auto reason = reasoning.Reason(
        user,
        "Should we ship this change now?",
        {"all tests are green", "the fix is narrow"},
        {"canary still pending", "owner may be unavailable"},
        "Ship only after canary and owner signoff.");
    REQUIRE(reason.Ok());
    REQUIRE(reason.value.question.find("ship") != std::string::npos);
    REQUIRE(!reason.value.summary.empty());

    DecisionEngine decisions(&governance);
    auto rec = decisions.RecommendAction(
        user,
        "deploy",
        "Ship after canary and owner signoff",
        "ev://reasoning/ship-canary");
    REQUIRE(rec.Ok());
    REQUIRE(rec.value.submitted_to_governance);

    auto gate = decisions.AssertApproved(rec.value.governance_decision_id);
    REQUIRE(gate.error == CognitiveError::NotApproved);

    gov::PrincipalId owner{};
    owner.bytes[0] = 1;
    gov::DecisionId did{};
    did.bytes = rec.value.governance_decision_id;
    REQUIRE(governance.Approve(owner, true, did, "owner signoff").Ok());
    REQUIRE(decisions.AssertApproved(rec.value.governance_decision_id).Ok());

    ReflectionEngine reflections(&memory);
    auto note = reflections.Reflect(
        user,
        "decision-review",
        "The narrow fix is acceptable, but the deployment must wait for the canary window.");
    REQUIRE(note.Ok());
    REQUIRE(note.value.subject == "decision-review");

    governance.Close();
    memory.Close();
    std::filesystem::remove_all(gov_dir, ec);
    std::filesystem::remove_all(mem_dir, ec);
    std::cout << "cognitive_reasoning_engine_smoke_test: OK\n";
    return 0;
}
