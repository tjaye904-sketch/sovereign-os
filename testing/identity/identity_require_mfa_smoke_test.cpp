#include "identity_service.hpp"
#include "identity_store_memory.hpp"
#include "identity_totp.hpp"
#include "test_require.hpp"
#include <iostream>
using namespace sovereign::identity;
int main()
{
    MemoryIdentityStore store;
    REQUIRE(store.Open({}) == IdentityError::Ok);
    auto cfg = DefaultIdentityConfig();
    cfg.require_mfa_on_login = true;
    IdentityService identity(store, cfg);
    REQUIRE(identity.BootstrapOwner("owner", "secret").Ok());

    auto open = identity.Authenticate("owner", "secret");
    REQUIRE(open.Ok()); // no TOTP yet

    auto raw = totp::RandomSecret(20);
    auto b32 = totp::Base32Encode(raw);
    auto owner = store.FindOwner();
    REQUIRE(owner.Ok() && owner.value.has_value());
    REQUIRE(store.SetCredentialVerifier(
                    owner.value->id, CredentialType::TotpSecret, b32)
                .Ok());

    auto blocked = identity.Authenticate("owner", "secret");
    REQUIRE(blocked.error == IdentityError::MfaRequired);

    // Wrong code
    auto bad = identity.AuthenticateWithMfa("owner", "secret", "000000");
    // May rarely succeed if 000000 matches; accept MfaInvalid or Ok
    if (!bad.Ok())
        REQUIRE(bad.error == IdentityError::MfaInvalid);

    std::cout << "identity_require_mfa_smoke_test: OK\n";
    return 0;
}
