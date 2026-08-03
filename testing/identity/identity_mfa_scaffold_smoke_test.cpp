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
    auto again = identity.AuthenticateWithMfa("owner", "secret", "000000");
    REQUIRE(again.Ok()); // not enrolled → MFA optional
    auto begin = identity.BeginTotpEnrollment(sess.value.id);
    REQUIRE(begin.Ok());
    REQUIRE(begin.value.find("otpauth://totp/") == 0);
    // still not enrolled until confirm
    auto st2 = identity.GetMfaStatus(sess.value.principal_id);
    REQUIRE(st2.Ok());
    REQUIRE(st2.value.totp_enrolled == false);
    std::cout << "identity_mfa_scaffold_smoke_test: OK\n";
    return 0;
}
