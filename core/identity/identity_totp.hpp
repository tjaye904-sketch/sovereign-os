#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::identity::totp
{

std::string Base32Encode(const std::vector<std::uint8_t>& raw);
std::vector<std::uint8_t> Base32Decode(const std::string& b32);
std::vector<std::uint8_t> RandomSecret(std::size_t n = 20);
std::string ProvisioningUri(const std::string& b32_secret, const std::string& account);
// RFC 6238 TOTP SHA1, 30s step, 6 digits; accepts ±1 window
bool VerifyCode(const std::string& b32_secret, const std::string& code, std::int64_t now_unix);

} // namespace sovereign::identity::totp
