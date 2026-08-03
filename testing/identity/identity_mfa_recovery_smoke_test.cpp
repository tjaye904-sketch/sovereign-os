#include "identity_service.hpp"
#include "identity_store_memory.hpp"
#include "test_require.hpp"
#include <iostream>
using namespace sovereign::identity;
int main()
{
    MemoryIdentityStore store;
    REQUIRE(store.Open({}) == IdentityError::Ok);
    IdentityService identity(store, DefaultIdentityConfig());
    REQUIRE(identity.BootstrapOwner("owner", "secret").Ok());
    auto sess = identity.Authenticate("owner", "secret");
    REQUIRE(sess.Ok());
    auto codes = identity.GenerateRecoveryCodes(sess.value.id);
    REQUIRE(codes.Ok());
    REQUIRE(codes.value.size() == 8);
    auto ok = identity.ConsumeRecoveryCode(sess.value.principal_id, codes.value[0]);
    REQUIRE(ok.Ok());
    auto again = identity.ConsumeRecoveryCode(sess.value.principal_id, codes.value[0]);
    REQUIRE(!again.Ok()); // one-time
    // step-up with no MFA enrolled → ok
    auto su = identity.VerifyStepUp(sess.value.id, "irrelevant");
    REQUIRE(su.Ok());
    std::cout << "identity_mfa_recovery_smoke_test: OK\n";
    return 0;
}
