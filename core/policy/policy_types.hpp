#pragma once

#include "principal_id.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::policy
{

using ::sovereign::PrincipalId;

enum class PolicyEffect : std::uint8_t
{
    Allow = 1,
    Deny = 2
};

struct PolicyDecision
{
    PolicyEffect effect{PolicyEffect::Deny};
    std::string reason;
    std::uint64_t policy_version{0};
    bool dry_run{false};
};

struct PolicyRule
{
    std::string action; // exact match; "*" = default
    std::string min_role; // owner | operator | service | readonly
    PolicyEffect effect{PolicyEffect::Allow};
};

enum class PolicyError : std::uint8_t
{
    Ok = 0,
    InvalidArgument,
    StorageFailure
};

template <typename T>
struct Result
{
    PolicyError error{PolicyError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == PolicyError::Ok;
    }
};

} // namespace sovereign::policy
