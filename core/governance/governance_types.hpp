#pragma once

#include "principal_id.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::governance
{

using ::sovereign::PrincipalId;
using ::sovereign::SessionId;

struct DecisionId
{
    std::array<std::uint8_t, 16> bytes{};
};

inline bool operator==(const DecisionId& a, const DecisionId& b) noexcept
{
    return a.bytes == b.bytes;
}

enum class DecisionStatus : std::uint8_t
{
    Pending = 1,
    Approved = 2,
    Rejected = 3,
    Cancelled = 4
};

struct Decision
{
    DecisionId id{};
    PrincipalId requester{};
    std::string action;
    std::string summary;
    std::string evidence_ref;
    DecisionStatus status{DecisionStatus::Pending};
    PrincipalId resolved_by{};
    std::string resolution_reason;
    std::uint64_t created_unix_seconds{0};
    std::uint64_t resolved_unix_seconds{0};
    bool is_override{false};
};

enum class GovernanceError : std::uint8_t
{
    Ok = 0,
    NotFound,
    InvalidArgument,
    Unauthorized,
    InvalidState,
    StorageFailure
};

template <typename T>
struct Result
{
    GovernanceError error{GovernanceError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == GovernanceError::Ok;
    }
};

template <>
struct Result<void>
{
    GovernanceError error{GovernanceError::Ok};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == GovernanceError::Ok;
    }
};

} // namespace sovereign::governance
