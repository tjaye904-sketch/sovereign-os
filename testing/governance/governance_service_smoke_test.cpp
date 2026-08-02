#include "governance_service.hpp"
#include "test_require.hpp"
#include <filesystem>
#include <iostream>

using namespace sovereign::governance;

int main()
{
    const auto dir =
        std::filesystem::temp_directory_path() / "sovereign_gov_test";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    std::filesystem::create_directories(dir, ec);

    PrincipalId requester{};
    requester.bytes[0] = 1;
    PrincipalId owner{};
    owner.bytes[0] = 2;

    DecisionId id{};
    {
        GovernanceService gov;
        REQUIRE(gov.Open(dir) == GovernanceError::Ok);
        auto req = gov.RequestDecision(
            requester, "deploy", "ship v1", "evidence://1");
        REQUIRE(req.Ok());
        id = req.value;
        auto pending = gov.ListPending();
        REQUIRE(pending.Ok());
        REQUIRE(pending.value.size() == 1);

        auto denied = gov.Approve(requester, false, id, "no");
        REQUIRE(denied.error == GovernanceError::Unauthorized);

        REQUIRE(gov.Approve(owner, true, id, "looks good").Ok());
        auto got = gov.Get(id);
        REQUIRE(got.Ok());
        REQUIRE(got.value.status == DecisionStatus::Approved);
        gov.Close();
    }
    {
        GovernanceService gov;
        REQUIRE(gov.Open(dir) == GovernanceError::Ok);
        auto got = gov.Get(id);
        REQUIRE(got.Ok());
        REQUIRE(got.value.status == DecisionStatus::Approved);
        gov.Close();
    }
    std::filesystem::remove_all(dir, ec);
    std::cout << "governance_service_smoke_test: OK\n";
    return 0;
}
