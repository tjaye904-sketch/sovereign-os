#pragma once

#include "principal_id.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::audit
{

using ::sovereign::PrincipalId;

struct AuditEntryId
{
    std::array<std::uint8_t, 16> bytes{};
};

struct AuditEntry
{
    AuditEntryId id{};
    std::uint64_t unix_seconds{0};
    std::string category; // identity | governance | policy | executor | system
    std::string action;
    PrincipalId actor{};
    std::string subject; // free-form id or label (never secrets)
    std::string detail;  // free-form, never secrets
};

enum class AuditError : std::uint8_t
{
    Ok = 0,
    InvalidArgument,
    StorageFailure
};

template <typename T>
struct Result
{
    AuditError error{AuditError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == AuditError::Ok;
    }
};

} // namespace sovereign::audit
