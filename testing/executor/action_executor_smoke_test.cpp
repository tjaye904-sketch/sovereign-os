#include "action_executor.hpp"
#include "cognitive_service.hpp"
#include "governance_service.hpp"
#include "test_require.hpp"
#include <filesystem>
#include <iostream>

int main()
{
    using namespace sovereign::executor;
    namespace gov = sovereign::governance;
    namespace cog = sovereign::cognitive;

    const auto dir = std::filesystem::temp_directory_path() / "sov_exec_test";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);

    gov::GovernanceService governance;
    REQUIRE(governance.Open(dir) == gov::GovernanceError::Ok);
    cog::CognitiveService cognitive(governance, nullptr);
    ActionExecutor executor(cognitive);

    int calls = 0;
    executor.RegisterHandler("deploy", [&](const std::string&) {
        Result<std::string> r;
        r.value = "deployed";
        ++calls;
        return r;
    });

    sovereign::PrincipalId user{};
    user.bytes[0] = 3;
    auto rec = cognitive.RecommendAction(user, "deploy", "ship", "ev");
    REQUIRE(rec.Ok());

    auto denied = executor.Execute("deploy", rec.value.governance_decision_id);
    REQUIRE(denied.error == ExecutorError::NotApproved);
    REQUIRE(calls == 0);

    gov::PrincipalId owner{};
    owner.bytes[0] = 1;
    gov::DecisionId did{};
    did.bytes = rec.value.governance_decision_id;
    REQUIRE(governance.Approve(owner, true, did, "yes").Ok());

    auto ok = executor.Execute("deploy", rec.value.governance_decision_id);
    REQUIRE(ok.Ok());
    REQUIRE(ok.value.succeeded);
    REQUIRE(calls == 1);

    governance.Close();
    std::filesystem::remove_all(dir, ec);
    std::cout << "action_executor_smoke_test: OK\n";
    return 0;
}
