#pragma once

#include "identity_store.hpp"

#include <mutex>
#include <unordered_map>
#include <utility>

namespace sovereign::identity
{

class MemoryIdentityStore final : public IdentityStore
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
    static std::string IdKey(const PrincipalId& id);
    static std::string IdKey(const SessionId& id);

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path directory_;
    std::unordered_map<std::string, Principal> principals_;
    std::unordered_map<std::string, std::string> verifiers_; // key: id+type
    std::unordered_map<std::string, std::vector<std::string>> roles_;
    std::unordered_map<std::string, Session> sessions_;
};

} // namespace sovereign::identity
