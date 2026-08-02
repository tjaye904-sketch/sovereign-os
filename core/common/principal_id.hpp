#pragma once

// Shared kernel identity value types.
// Used by Identity, Governance, Policy, and future product services.

#include <array>
#include <cstdint>
#include <string>

namespace sovereign
{

struct PrincipalId
{
    std::array<std::uint8_t, 16> bytes{};
};

struct SessionId
{
    std::array<std::uint8_t, 16> bytes{};
};

inline bool operator==(const PrincipalId& a, const PrincipalId& b) noexcept
{
    return a.bytes == b.bytes;
}

inline bool operator!=(const PrincipalId& a, const PrincipalId& b) noexcept
{
    return !(a == b);
}

inline bool operator==(const SessionId& a, const SessionId& b) noexcept
{
    return a.bytes == b.bytes;
}

inline bool operator!=(const SessionId& a, const SessionId& b) noexcept
{
    return !(a == b);
}

inline bool IsNil(const PrincipalId& id) noexcept
{
    for (auto b : id.bytes)
    {
        if (b != 0)
        {
            return false;
        }
    }
    return true;
}

inline bool IsNil(const SessionId& id) noexcept
{
    for (auto b : id.bytes)
    {
        if (b != 0)
        {
            return false;
        }
    }
    return true;
}

inline std::string ToHex(const std::array<std::uint8_t, 16>& bytes)
{
    static constexpr char kHex[] = "0123456789abcdef";
    std::string out(32, '0');
    for (std::size_t i = 0; i < 16; ++i)
    {
        out[i * 2] = kHex[(bytes[i] >> 4) & 0xF];
        out[i * 2 + 1] = kHex[bytes[i] & 0xF];
    }
    return out;
}

inline bool FromHex(const std::string& hex, std::array<std::uint8_t, 16>& bytes)
{
    if (hex.size() != 32)
    {
        return false;
    }
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9')
        {
            return c - '0';
        }
        if (c >= 'a' && c <= 'f')
        {
            return c - 'a' + 10;
        }
        if (c >= 'A' && c <= 'F')
        {
            return c - 'A' + 10;
        }
        return -1;
    };
    for (std::size_t i = 0; i < 16; ++i)
    {
        const int hi = nibble(hex[i * 2]);
        const int lo = nibble(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0)
        {
            return false;
        }
        bytes[i] = static_cast<std::uint8_t>((hi << 4) | lo);
    }
    return true;
}

} // namespace sovereign
