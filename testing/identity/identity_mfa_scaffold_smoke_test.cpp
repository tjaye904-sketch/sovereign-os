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
    auto st = identity.GetMfaStatus(sess.value.principal_id);
    REQUIRE(st.Ok());
    REQUIRE(st.value.totp_enrolled == false);
    // existing login path unchanged
    auto again = identity.AuthenticateWithMfa("owner", "secret", "000000");
    REQUIRE(again.Ok());
    auto begin = identity.BeginTotpEnrollment(sess.value.id);
    REQUIRE(begin.error == IdentityError::MfaNotEnrolled);
    std::cout << "identity_mfa_scaffold_smoke_test: OK\n";
    return 0;
}
