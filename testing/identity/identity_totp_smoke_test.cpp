#include "identity_totp.hpp"
#include "test_require.hpp"
#include <chrono>
#include <iostream>
using namespace sovereign::identity;
int main()
{
    auto raw = totp::RandomSecret(20);
    auto b32 = totp::Base32Encode(raw);
    REQUIRE(!b32.empty());
    auto dec = totp::Base32Decode(b32);
    REQUIRE(dec.size() == raw.size());
    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();
    // deterministic: empty secret fails
    REQUIRE(!totp::VerifyCode("", "123456", now));
    auto uri = totp::ProvisioningUri(b32, "owner");
    REQUIRE(uri.find("otpauth://totp/") == 0);
    std::cout << "identity_totp_smoke_test: OK\n";
    return 0;
}
