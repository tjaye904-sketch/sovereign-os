// Milestone A.1 — session/principal revoke & suspend fully verified
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

    REQUIRE(identity.BootstrapOwner("owner", "owner-secret").Ok());
    auto owner_sess = identity.Authenticate("owner", "owner-secret");
    REQUIRE(owner_sess.Ok());

    // --- Session revoke ---
    auto s = identity.Authenticate("owner", "owner-secret");
    REQUIRE(s.Ok());
    REQUIRE(identity.ValidateSession(s.value.id).Ok());
    REQUIRE(identity.RevokeSession(s.value.id).Ok());
    REQUIRE(
        identity.ValidateSession(s.value.id).error ==
        IdentityError::SessionRevoked);
    // Double-revoke is still Ok or already revoked — must not crash
    static_cast<void>(identity.RevokeSession(s.value.id));

    // --- Operator suspend blocks new auth ---
    auto op = identity.CreateOperator(
        owner_sess.value.id, "op1", "op-secret");
    REQUIRE(op.Ok());
    auto op_login = identity.Authenticate("op1", "op-secret");
    REQUIRE(op_login.Ok());
    REQUIRE(identity
                .SetPrincipalStatus(
                    owner_sess.value.id, op.value, PrincipalStatus::Suspended)
                .Ok());
    auto op_blocked = identity.Authenticate("op1", "op-secret");
    REQUIRE(op_blocked.error == IdentityError::PrincipalSuspended);

    // --- Operator revoke blocks auth ---
    auto op2 = identity.CreateOperator(
        owner_sess.value.id, "op2", "op2-secret");
    REQUIRE(op2.Ok());
    REQUIRE(identity.Authenticate("op2", "op2-secret").Ok());
    REQUIRE(identity
                .SetPrincipalStatus(
                    owner_sess.value.id, op2.value, PrincipalStatus::Revoked)
                .Ok());
    auto op2_blocked = identity.Authenticate("op2", "op2-secret");
    REQUIRE(op2_blocked.error == IdentityError::PrincipalRevoked);

    // --- Non-owner cannot suspend (operator session) ---
    // Refresh owner session (previous still valid)
    auto owner2 = identity.Authenticate("owner", "owner-secret");
    REQUIRE(owner2.Ok());
    auto op3 = identity.CreateOperator(
        owner2.value.id, "op3", "op3-secret");
    REQUIRE(op3.Ok());
    auto op3_sess = identity.Authenticate("op3", "op3-secret");
    REQUIRE(op3_sess.Ok());
    auto denied = identity.SetPrincipalStatus(
        op3_sess.value.id, op3.value, PrincipalStatus::Suspended);
    REQUIRE(!denied.Ok());

    // --- Owner remains authenticable after operator changes ---
    REQUIRE(identity.Authenticate("owner", "owner-secret").Ok());

    std::cout << "identity_revocation_hardening_smoke_test: OK\n";
    return 0;
}
