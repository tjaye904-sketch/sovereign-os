#include "identity_service.hpp"
#include "identity_store_memory.hpp"
#include "test_require.hpp"

#include <iostream>

using namespace sovereign::identity;

int main()
{
    MemoryIdentityStore store;
    REQUIRE(store.Open({}) == IdentityError::Ok);
    IdentityConfig config = DefaultIdentityConfig();
    config.session_lifetime_seconds = 3600;
    IdentityService identity(store, config);

    REQUIRE(identity.BootstrapOwner("owner", "s1").Ok());
    auto s1 = identity.Authenticate("owner", "s1");
    REQUIRE(s1.Ok());

    // Suspend path via status (owner session)
    auto op = identity.CreateOperator(s1.value.id, "op", "op-secret");
    REQUIRE(op.Ok());
    auto op_s = identity.Authenticate("op", "op-secret");
    REQUIRE(op_s.Ok());

    REQUIRE(identity.SetPrincipalStatus(
                s1.value.id, op.value, PrincipalStatus::Suspended)
                .Ok());
    auto blocked = identity.Authenticate("op", "op-secret");
    REQUIRE(blocked.error == IdentityError::PrincipalSuspended);

    // Revoked session after revoke
    auto s2 = identity.Authenticate("owner", "s1");
    REQUIRE(s2.Ok());
    REQUIRE(identity.RevokeSession(s2.value.id).Ok());
    REQUIRE(
        identity.ValidateSession(s2.value.id).error ==
        IdentityError::SessionRevoked);

    // Refresh extends lifetime
    auto s3 = identity.Authenticate("owner", "s1");
    REQUIRE(s3.Ok());
    const auto exp_before = s3.value.expires_unix_seconds;
    auto refreshed = identity.RefreshSession(s3.value.id);
    REQUIRE(refreshed.Ok());
    REQUIRE(refreshed.value.expires_unix_seconds >= exp_before);

    // Empty args
    REQUIRE(
        identity.BootstrapOwner("other", "x").error ==
        IdentityError::AlreadyBootstrapped);
    REQUIRE(
        identity.Authenticate("", "x").error ==
        IdentityError::InvalidArgument);

    std::cout << "identity_acceptance_smoke_test: OK\n";
    return 0;
}
