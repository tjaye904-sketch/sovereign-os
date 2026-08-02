#pragma once

#include "identity_store.hpp"

#include <mutex>

namespace sovereign::identity
{

// Durable local store (IQ-1A session survival across process restart).
// Layout under data_directory:
//   principals.dat  verifiers.dat  roles.dat  sessions.dat
class FileIdentityStore final : public IdentityStore
{
public:
    IdentityError Open(const std::filesystem::path& directory) override;
    void Close() noexcept override;

    Result<PrincipalId> InsertPrincipal(const Principal& principal) override;
    Result<Principal> GetPrincipal(const PrincipalId& id) const override;
    Result<std::vector<Principal>> ListPrincipals() const override;
    Result<void> UpdatePrincipalStatus(
        const PrincipalId& id,
        PrincipalStatus status) override;

    Result<void> SetCredentialVerifier(
        const PrincipalId& id,
        CredentialType type,
        const std::string& verifier) override;
    Result<std::string> GetCredentialVerifier(
        const PrincipalId& id,
        CredentialType type) const override;

    Result<void> AssignRole(
        const PrincipalId& id,
        const std::string& role) override;
    Result<void> RevokeRole(
        const PrincipalId& id,
        const std::string& role) override;
    Result<std::vector<std::string>> GetRoles(
        const PrincipalId& id) const override;

    Result<SessionId> InsertSession(const Session& session) override;
    Result<Session> GetSession(const SessionId& id) const override;
    Result<std::vector<Session>> ListSessions(
        const PrincipalId& principal_id) const override;
    Result<void> UpdateSession(const Session& session) override;
    Result<void> RevokeAllSessions(const PrincipalId& principal_id) override;

    Result<std::optional<Principal>> FindOwner() const override;

private:
    IdentityError LoadAll();
    IdentityError SaveAll() const;

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path directory_;
    std::vector<Principal> principals_;
    // verifier key: 16-byte id + 1 byte type
    struct VerifierRecord
    {
        PrincipalId id{};
        CredentialType type{CredentialType::LocalSecret};
        std::string verifier;
    };
    std::vector<VerifierRecord> verifiers_;
    struct RoleRecord
    {
        PrincipalId id{};
        std::string role;
    };
    std::vector<RoleRecord> roles_;
    std::vector<Session> sessions_;
};

} // namespace sovereign::identity
