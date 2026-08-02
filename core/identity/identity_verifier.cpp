#include "identity_verifier.hpp"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// Argon2 C API (libargon2). Declared so builds work without argon2.h.
extern "C" {
int argon2id_hash_encoded(
    const std::uint32_t t_cost,
    const std::uint32_t m_cost,
    const std::uint32_t parallelism,
    const void* pwd,
    const std::size_t pwdlen,
    const void* salt,
    const std::size_t saltlen,
    const std::size_t hashlen,
    char* encoded,
    const std::size_t encodedlen);

int argon2id_verify(
    const char* encoded,
    const void* pwd,
    const std::size_t pwdlen);
}

namespace sovereign::identity
{
namespace
{

struct Argon2Params
{
    std::uint32_t time_cost{3};
    std::uint32_t memory_kib{65536};
    std::uint32_t parallelism{1};
};

std::uint64_t ReadMemTotalKib()
{
    std::ifstream in("/proc/meminfo");
    std::string key;
    std::uint64_t value = 0;
    std::string unit;
    while (in >> key >> value >> unit)
    {
        if (key == "MemTotal:")
        {
            return value; // already KiB on Linux
        }
    }
    return 8ULL * 1024ULL * 1024ULL; // assume 8 GiB if unknown
}

Argon2Params TuneForHardware()
{
    Argon2Params p;
    const unsigned hw = std::thread::hardware_concurrency();
    const std::uint64_t mem_kib = ReadMemTotalKib();

    // Parallelism: use up to 4 threads, at least 1, not more than cores.
    p.parallelism = 1;
    if (hw >= 2)
    {
        p.parallelism = std::min(4u, hw);
    }

    // Memory: aim ~1/32 of RAM, clamped [32 MiB, 256 MiB].
    // Leave headroom for other processes on small hosts.
    std::uint64_t target = mem_kib / 32;
    if (target < 32ULL * 1024ULL)
    {
        target = 32ULL * 1024ULL;
    }
    if (target > 256ULL * 1024ULL)
    {
        target = 256ULL * 1024ULL;
    }
    // On hosts with <3 GiB RAM, prefer 32 MiB to avoid pressure.
    if (mem_kib < 3ULL * 1024ULL * 1024ULL)
    {
        target = 32ULL * 1024ULL;
    }
    p.memory_kib = static_cast<std::uint32_t>(target);

    // Time cost: slightly higher on strong hosts.
    p.time_cost = (mem_kib >= 8ULL * 1024ULL * 1024ULL && hw >= 4) ? 3u : 2u;

    // Env overrides (owner tuning)
    if (const char* t = std::getenv("SOVEREIGN_ARGON2_T"); t && t[0])
    {
        p.time_cost = static_cast<std::uint32_t>(std::strtoul(t, nullptr, 10));
    }
    if (const char* m = std::getenv("SOVEREIGN_ARGON2_M"); m && m[0])
    {
        p.memory_kib = static_cast<std::uint32_t>(std::strtoul(m, nullptr, 10));
    }
    if (const char* par = std::getenv("SOVEREIGN_ARGON2_P"); par && par[0])
    {
        p.parallelism =
            static_cast<std::uint32_t>(std::strtoul(par, nullptr, 10));
    }
    if (p.time_cost < 1)
    {
        p.time_cost = 1;
    }
    if (p.memory_kib < 16 * 1024)
    {
        p.memory_kib = 16 * 1024;
    }
    if (p.parallelism < 1)
    {
        p.parallelism = 1;
    }
    return p;
}

const Argon2Params& Params()
{
    static const Argon2Params kParams = TuneForHardware();
    return kParams;
}

constexpr std::size_t kHashLen = 32;
constexpr std::size_t kSaltLen = 16;
constexpr std::size_t kEncodedMax = 256;

std::string HashV1Insecure(const std::string& secret)
{
    constexpr const char* kScheme = "sov-id-v1";
    std::string material = std::string(kScheme) + ":" + secret;
    std::uint64_t h = 14695981039346656037ull;
    for (int round = 0; round < 10000; ++round)
    {
        for (unsigned char c : material)
        {
            h ^= static_cast<std::uint64_t>(c);
            h *= 1099511628211ull;
        }
        material = std::to_string(h) + material;
        if (material.size() > 256)
        {
            material.resize(256);
        }
    }
    return std::string(kScheme) + ":" + std::to_string(h);
}

std::string HashArgon2id(const std::string& secret)
{
    const auto& p = Params();
    std::array<std::uint8_t, kSaltLen> salt{};
    std::random_device rd;
    for (auto& b : salt)
    {
        b = static_cast<std::uint8_t>(rd());
    }

    std::string encoded(kEncodedMax, '\0');
    const int rc = argon2id_hash_encoded(
        p.time_cost,
        p.memory_kib,
        p.parallelism,
        secret.data(),
        secret.size(),
        salt.data(),
        salt.size(),
        kHashLen,
        encoded.data(),
        encoded.size());
    if (rc != 0)
    {
        throw std::runtime_error("argon2id_hash_encoded failed");
    }
    encoded.resize(std::strlen(encoded.c_str()));
    return std::string(kVerifierSchemeArgon2id) + ":" + encoded;
}

bool VerifyArgon2id(const std::string& stored, const std::string& secret)
{
    const std::string prefix = std::string(kVerifierSchemeArgon2id) + ":";
    if (stored.rfind(prefix, 0) != 0)
    {
        return false;
    }
    const char* encoded = stored.c_str() + prefix.size();
    return argon2id_verify(encoded, secret.data(), secret.size()) == 0;
}

bool VerifyV1(const std::string& stored, const std::string& secret)
{
    return stored == HashV1Insecure(secret);
}

} // namespace

std::string DescribeArgon2Params()
{
    const auto& p = Params();
    return "t=" + std::to_string(p.time_cost) +
           " m_kib=" + std::to_string(p.memory_kib) +
           " p=" + std::to_string(p.parallelism);
}

std::string HashSecret(const std::string& secret)
{
    try
    {
        return HashArgon2id(secret);
    }
    catch (...)
    {
        if (!AllowInsecureVerifier())
        {
            throw;
        }
        return HashV1Insecure(secret);
    }
}

bool VerifySecret(const std::string& stored_verifier, const std::string& secret)
{
    if (stored_verifier.rfind(kVerifierSchemeArgon2id, 0) == 0)
    {
        return VerifyArgon2id(stored_verifier, secret);
    }
    if (stored_verifier.rfind(kVerifierSchemeV1, 0) == 0)
    {
        if (!AllowInsecureVerifier())
        {
            return false;
        }
        return VerifyV1(stored_verifier, secret);
    }
    return false;
}

} // namespace sovereign::identity
