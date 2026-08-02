#include "cognitive_service.hpp"
#include "governance_service.hpp"
#include "test_require.hpp"
#include <filesystem>
#include <iostream>

int main()
{
    using namespace sovereign::cognitive;
    namespace gov = sovereign::governance;

    const auto dir =
        std::filesystem::temp_directory_path() / "sov_cog_gov";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);

    gov::GovernanceService governance;
    REQUIRE(governance.Open(dir) == gov::GovernanceError::Ok);

    CognitiveService cognitive(governance, nullptr);

    PrincipalId user{};
    user.bytes[0] = 9;

    auto curiosity = cognitive.ProposeCuriosity(
        user, "battery", {"what chemistry?", "what cost?"});
    REQUIRE(curiosity.Ok());
    REQUIRE(curiosity.value.questions.size() == 2);

    auto reason = cognitive.Reason(
        user,
        "deploy now?",
        {"tests green"},
        {"no canary"},
        "wait for canary");
    REQUIRE(reason.Ok());

    auto rec = cognitive.RecommendAction(
        user, "deploy", "ship build", "ev://1");
    REQUIRE(rec.Ok());
    REQUIRE(rec.value.submitted_to_governance);

    // Not approved yet
    auto gate = cognitive.AssertApproved(rec.value.governance_decision_id);
    REQUIRE(gate.error == CognitiveError::NotApproved);

    // Owner approves
    gov::PrincipalId owner{};
    owner.bytes[0] = 1;
    gov::DecisionId did{};
    did.bytes = rec.value.governance_decision_id;
    REQUIRE(governance.Approve(owner, true, did, "ok").Ok());

    REQUIRE(cognitive.AssertApproved(rec.value.governance_decision_id).Ok());

    governance.Close();
    std::filesystem::remove_all(dir, ec);
    std::cout << "cognitive_governance_smoke_test: OK\n";
    return 0;
}
