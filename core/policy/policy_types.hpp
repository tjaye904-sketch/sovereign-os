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
};

enum class PolicyError : std::uint8_t
{
    Ok = 0,
    InvalidArgument
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
