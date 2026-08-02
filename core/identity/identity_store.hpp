#pragma once

#include "identity_types.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace sovereign::identity
{

// v1 storage port. SQLite implementation can follow; in-memory for first tests.
class IdentityStore
{
public:
    virtual ~IdentityStore() = default;

    virtual IdentityError Open(const std::filesystem::path& directory) = 0;
    virtual void Close() noexcept = 0;

    virtual Result<PrincipalId> InsertPrincipal(const Principal& principal) = 0;
    virtual Result<Principal> GetPrincipal(const PrincipalId& id) const = 0;
    virtual Result<std::vector<Principal>> ListPrincipals() const = 0;
    virtual Result<void> UpdatePrincipalStatus(
        const PrincipalId& id,
        PrincipalStatus status) = 0;

    virtual Result<void> SetCredentialVerifier(
        const PrincipalId& id,
        CredentialType type,
        const std::string& verifier) = 0;
    virtual Result<std::string> GetCredentialVerifier(
        const PrincipalId& id,
        CredentialType type) const = 0;

    virtual Result<void> AssignRole(
        const PrincipalId& id,
        const std::string& role) = 0;
    virtual Result<void> RevokeRole(
        const PrincipalId& id,
        const std::string& role) = 0;
    virtual Result<std::vector<std::string>> GetRoles(
        const PrincipalId& id) const = 0;

    virtual Result<SessionId> InsertSession(const Session& session) = 0;
    virtual Result<Session> GetSession(const SessionId& id) const = 0;
    virtual Result<std::vector<Session>> ListSessions(
        const PrincipalId& principal_id) const = 0;
    virtual Result<void> UpdateSession(const Session& session) = 0;
    virtual Result<void> RevokeAllSessions(const PrincipalId& principal_id) = 0;

    virtual Result<std::optional<Principal>> FindOwner() const = 0;
};

} // namespace sovereign::identity
