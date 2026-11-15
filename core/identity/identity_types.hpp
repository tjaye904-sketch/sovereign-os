#pragma once

// Product Identity Service types.
// PrincipalId / SessionId come from core/common (shared kernel types).

#include "principal_id.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::identity
{

using ::sovereign::PrincipalId;
using ::sovereign::SessionId;
using ::sovereign::IsNil;

enum class PrincipalKind : std::uint8_t
{
    Owner = 1,
    Operator = 2,
    Service = 3, // reserved in schema; no public auth API in v1
    Device = 4
};

enum class PrincipalStatus : std::uint8_t
{
    Active = 1,
    Suspended = 2,
    Revoked = 3
};

enum class SessionStatus : std::uint8_t
{
    Active = 1,
    Expired = 2,
    Revoked = 3
};

enum class MfaFactorKind : std::uint8_t
{
    None = 0,
    Totp = 1,
    RecoveryCode = 2
};

enum class CredentialType : std::uint8_t
{
    LocalSecret = 1,
    TotpSecret = 2,
    RecoveryCodes = 3,
    LocalToken = 4
};

struct Principal
{
    PrincipalId id{};
    PrincipalKind kind{PrincipalKind::Operator};
    PrincipalStatus status{PrincipalStatus::Active};
    std::string display_name;
    std::uint64_t created_unix_seconds{0};
    std::uint64_t updated_unix_seconds{0};
};

struct Session
{
    SessionId id{};
    PrincipalId principal_id{};
    SessionStatus status{SessionStatus::Active};
    std::uint64_t issued_unix_seconds{0};
    std::uint64_t last_activity_unix_seconds{0};
    std::uint64_t expires_unix_seconds{0};
    std::string origin_label;
    CredentialType auth_method{CredentialType::LocalSecret};
};

inline constexpr const char* kRoleOwner = "owner";
inline constexpr const char* kRoleOperator = "operator";
inline constexpr const char* kRoleReadonly = "readonly";
inline constexpr const char* kRoleService = "service";

enum class IdentityError : std::uint8_t
{
    Ok = 0,
    NotFound,
    AlreadyExists,
    InvalidCredential,
    SessionExpired,
    SessionRevoked,
    PrincipalSuspended,
    PrincipalRevoked,
    NotBootstrapped,
    AlreadyBootstrapped,
    InvalidArgument,
    StorageFailure,
    Unauthorized,
    MfaRequired,
    MfaNotEnrolled,
    MfaInvalid
};

template <typename T>
struct Result
{
    IdentityError error{IdentityError::Ok};
    T value{};

    [[nodiscard]] bool Ok() const noexcept
    {
        return error == IdentityError::Ok;
    }
};

template <>
struct Result<void>
{
    IdentityError error{IdentityError::Ok};

    [[nodiscard]] bool Ok() const noexcept
    {
        return error == IdentityError::Ok;
    }
};

inline const char* IdentityErrorToString(IdentityError error) noexcept
{
    switch (error)
    {
        case IdentityError::Ok:
            return "ok";
        case IdentityError::NotFound:
            return "principal_not_found";
        case IdentityError::AlreadyExists:
            return "already_exists";
        case IdentityError::InvalidCredential:
            return "invalid_credential";
        case IdentityError::SessionExpired:
            return "session_expired";
        case IdentityError::SessionRevoked:
            return "session_revoked";
        case IdentityError::PrincipalSuspended:
            return "principal_suspended";
        case IdentityError::PrincipalRevoked:
            return "principal_revoked";
        case IdentityError::NotBootstrapped:
            return "not_bootstrapped";
        case IdentityError::AlreadyBootstrapped:
            return "already_bootstrapped";
        case IdentityError::InvalidArgument:
            return "invalid_argument";
        case IdentityError::StorageFailure:
            return "storage_failure";
        case IdentityError::Unauthorized:
            return "unauthorized";
        case IdentityError::MfaRequired:
            return "mfa_required";
        case IdentityError::MfaNotEnrolled:
            return "mfa_not_enrolled";
        case IdentityError::MfaInvalid:
            return "mfa_invalid";
        default:
            return "unknown_error";
    }
}

} // namespace sovereign::identity
