#include "identity_service.hpp"
#include "identity_store_memory.hpp"
#include "governance_service.hpp"
#include "test_require.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    using namespace sovereign::identity;
    namespace gov = sovereign::governance;

    MemoryIdentityStore store;
    REQUIRE(store.Open({}) == IdentityError::Ok);
    IdentityService id(store, DefaultIdentityConfig());
    REQUIRE(id.BootstrapOwner("owner", "secret").Ok());
    auto session = id.Authenticate("owner", "secret");
    REQUIRE(session.Ok());
    auto principal = id.GetPrincipal(session.value.principal_id);
    REQUIRE(principal.Ok());
    auto roles = id.GetRoles(principal.value.id);
    REQUIRE(roles.Ok());
    bool is_owner = false;
    for (const auto& r : roles.value)
        if (r == kRoleOwner) is_owner = true;
    REQUIRE(is_owner);

    gov::GovernanceService govsvc;
    const auto dir =
        std::filesystem::temp_directory_path() / "sov_id_gov_int";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    REQUIRE(govsvc.Open(dir) == gov::GovernanceError::Ok);

    gov::PrincipalId requester{};
    for (std::size_t i = 0; i < 16; ++i)
        requester.bytes[i] = principal.value.id.bytes[i];

    auto dec = govsvc.RequestDecision(
        requester, "deploy", "release", "ev-1");
    REQUIRE(dec.Ok());
    REQUIRE(govsvc.Approve(requester, is_owner, dec.value, "ok").Ok());
    auto got = govsvc.Get(dec.value);
    REQUIRE(got.Ok());
    REQUIRE(got.value.status == gov::DecisionStatus::Approved);
    govsvc.Close();
    std::filesystem::remove_all(dir, ec);
    std::cout << "identity_governance_integration_smoke_test: OK\n";
    return 0;
}
