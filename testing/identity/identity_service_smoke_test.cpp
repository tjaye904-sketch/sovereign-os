#include "identity_service.hpp"
#include "identity_store_memory.hpp"
#include "test_require.hpp"

#include <iostream>

using namespace sovereign::identity;

int main()
{
    REQUIRE(static_cast<int>(CredentialType::LocalSecret) !=
            static_cast<int>(CredentialType::TotpSecret));
    REQUIRE(static_cast<int>(CredentialType::TotpSecret) !=
            static_cast<int>(CredentialType::RecoveryCodes));
    REQUIRE(static_cast<int>(CredentialType::RecoveryCodes) !=
            static_cast<int>(CredentialType::LocalToken));
    REQUIRE(static_cast<int>(CredentialType::LocalToken) !=
            static_cast<int>(CredentialType::LocalSecret));

    MemoryIdentityStore store;
    REQUIRE(store.Open({}) == IdentityError::Ok);

    IdentityConfig config = DefaultIdentityConfig();
    config.session_lifetime_seconds = 3600;
    IdentityService identity(store, config);

    // Not bootstrapped yet
    auto early = identity.Authenticate("owner", "secret");
    REQUIRE(early.error == IdentityError::InvalidCredential);

    auto boot = identity.BootstrapOwner("owner", "secret");
    REQUIRE(boot.Ok());
    REQUIRE(!IsNil(boot.value));

    auto boot2 = identity.BootstrapOwner("owner2", "x");
    REQUIRE(boot2.error == IdentityError::AlreadyBootstrapped);

    auto bad = identity.Authenticate("owner", "wrong");
    REQUIRE(bad.error == IdentityError::InvalidCredential);

    auto session = identity.Authenticate("owner", "secret");
    REQUIRE(session.Ok());
    REQUIRE(session.value.status == SessionStatus::Active);

    auto valid = identity.ValidateSession(session.value.id);
    REQUIRE(valid.Ok());

    auto op = identity.CreateOperator(
        session.value.id,
        "alice",
        "alice-secret");
    REQUIRE(op.Ok());

    auto op_session = identity.Authenticate("alice", "alice-secret");
    REQUIRE(op_session.Ok());

    // Operator cannot create operators
    auto denied = identity.CreateOperator(
        op_session.value.id,
        "bob",
        "x");
    REQUIRE(denied.error == IdentityError::Unauthorized);

    auto roles = identity.GetRoles(boot.value);
    REQUIRE(roles.Ok());
    REQUIRE(!roles.value.empty());

    REQUIRE(identity.RevokeSession(session.value.id).Ok());
    auto after = identity.ValidateSession(session.value.id);
    REQUIRE(after.error == IdentityError::SessionRevoked);

    std::cout << "identity_service_smoke_test: OK\n";
    return 0;
}
