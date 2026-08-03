#pragma once

#include "identity_config.hpp"
#include "identity_store.hpp"
#include "identity_types.hpp"

#include <string>

namespace sovereign::identity
{

class IdentityService
{
public:
    IdentityService(IdentityStore& store, IdentityConfig config);

    Result<PrincipalId> BootstrapOwner(
        const std::string& display_name,
        const std::string& secret);

    Result<PrincipalId> CreateOperator(
        const SessionId& owner_session,
        const std::string& display_name,
        const std::string& secret);

    Result<PrincipalId> CreateServicePrincipal(
        const SessionId& owner_session,
        const std::string& display_name);

    Result<PrincipalId> CreateMachinePrincipal(
        const SessionId& owner_session,
        const std::string& display_name);

    Result<Principal> GetPrincipal(const PrincipalId& id) const;
    Result<std::vector<Principal>> ListPrincipals(
        const SessionId& requester_session) const;

    Result<void> SetPrincipalStatus(
        const SessionId& owner_session,
        const PrincipalId& id,
        PrincipalStatus status);

    Result<void> AssignRole(
        const SessionId& owner_session,
        const PrincipalId& id,
        const std::string& role);

    Result<void> RevokeRole(
        const SessionId& owner_session,
        const PrincipalId& id,
        const std::string& role);

    Result<std::vector<std::string>> GetRoles(const PrincipalId& id) const;

    Result<Session> Authenticate(
        const std::string& display_name,
        const std::string& secret);

    Result<Session> ValidateSession(const SessionId& session_id) const;
    Result<Session> RefreshSession(const SessionId& session_id);
    Result<void> RevokeSession(const SessionId& session_id);
    Result<std::vector<Session>> ListSessions(const PrincipalId& id) const;

    Result<void> ChangeCredential(
        const SessionId& session,
        const std::string& current_secret,
        const std::string& new_secret);

    // Milestone C — MFA scaffolding (public; login path unchanged until enrolled)
    struct MfaStatus
    {
        bool totp_enrolled{false};
        bool step_up_required_for_execute{false};
    };
    Result<MfaStatus> GetMfaStatus(const PrincipalId& id) const;
    Result<std::string> BeginTotpEnrollment(const SessionId& session);
    Result<void> ConfirmTotpEnrollment(const SessionId& session, const std::string& code);
    Result<Session> AuthenticateWithMfa(
        const std::string& display_name,
        const std::string& secret,
        const std::string& totp_code);

private:
    Result<Session> RequireActiveOwnerSession(const SessionId& session_id) const;
    static PrincipalId NewPrincipalId();
    static SessionId NewSessionId();
    static std::uint64_t NowUnixSeconds();

    IdentityStore& store_;
    IdentityConfig config_;
};

} // namespace sovereign::identity
