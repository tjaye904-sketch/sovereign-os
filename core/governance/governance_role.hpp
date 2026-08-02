#pragma once

// Adapted from Claude Sovereign evaluation (2026-07-30):
// human authority ranks, fail-safe to lowest authority.

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <string_view>

namespace sovereign::governance
{

enum class GovernanceRole : std::uint8_t
{
    Guest = 0,
    Operator = 1,       // maps TrustedUser / operator
    Administrator = 2,
    Owner = 3
};

[[nodiscard]] inline constexpr int Rank(GovernanceRole role) noexcept
{
    return static_cast<int>(role);
}

[[nodiscard]] inline constexpr bool AtLeast(
    GovernanceRole held,
    GovernanceRole required) noexcept
{
    return Rank(held) >= Rank(required);
}

// Fail-safe: unknown/missing → Guest
[[nodiscard]] inline GovernanceRole RoleFromString(std::string_view s)
{
    std::string n(s);
    std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    if (n == "owner")
        return GovernanceRole::Owner;
    if (n == "administrator" || n == "admin")
        return GovernanceRole::Administrator;
    if (n == "operator" || n == "trusted_user" || n == "trusteduser")
        return GovernanceRole::Operator;
    return GovernanceRole::Guest;
}

[[nodiscard]] inline const char* ToString(GovernanceRole r) noexcept
{
    switch (r)
    {
    case GovernanceRole::Owner:
        return "owner";
    case GovernanceRole::Administrator:
        return "administrator";
    case GovernanceRole::Operator:
        return "operator";
    default:
        return "guest";
    }
}

} // namespace sovereign::governance
