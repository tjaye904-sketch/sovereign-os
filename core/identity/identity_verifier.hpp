#pragma once

#include <cstdlib>
#include <string>

namespace sovereign::identity
{

// Verifier scheme ids (versioned for migration).
inline constexpr const char* kVerifierSchemeV1 = "sov-id-v1";
inline constexpr const char* kVerifierSchemeArgon2id = "sov-id-v2-argon2id";

// Production intent: Argon2id (approved 2026-07-29).
// Current build may lack argon2 headers; v1 iterative hash remains for tests
// and non-production only unless SOVEREIGN_ALLOW_INSECURE_VERIFIER=1.

[[nodiscard]] inline bool AllowInsecureVerifier()
{
#if defined(NDEBUG)
    const char* env = std::getenv("SOVEREIGN_ALLOW_INSECURE_VERIFIER");
    return env != nullptr && env[0] == '1';
#else
    return true; // Debug builds may use v1 for tests
#endif
}

[[nodiscard]] std::string HashSecret(const std::string& secret);

[[nodiscard]] bool VerifySecret(const std::string& stored_verifier, const std::string& secret);

[[nodiscard]] std::string DescribeArgon2Params();

} // namespace sovereign::identity
