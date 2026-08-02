#include "identity_service.hpp"
#include "identity_verifier.hpp"

#include <chrono>
#include <random>
#include <functional>

namespace sovereign::identity
{
namespace
{

template <typename Id>
Id RandomId()
{
    Id id{};
    std::random_device rd;
    for (auto& b : id.bytes)
    {
        b = static_cast<std::uint8_t>(rd());
    }
    return id;
}

} // namespace

IdentityService::IdentityService(IdentityStore& store, IdentityConfig config)
    : store_(store)
    , config_(std::move(config))
{
}

PrincipalId IdentityService::NewPrincipalId()
{
    return RandomId<PrincipalId>();
}

SessionId IdentityService::NewSessionId()
{
    return RandomId<SessionId>();
}

std::uint64_t IdentityService::NowUnixSeconds()
{
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(now).count());
}

Result<PrincipalId> IdentityService::BootstrapOwner(
    const std::string& display_name,
    const std::string& secret)
{
    Result<PrincipalId> result;
    if (display_name.empty() || secret.empty())
    {
        result.error = IdentityError::InvalidArgument;
        return result;
    }

    auto existing = store_.FindOwner();
    if (!existing.Ok())
    {
        result.error = existing.error;
        return result;
    }
    if (existing.value.has_value())
    {
        result.error = IdentityError::AlreadyBootstrapped;
        return result;
    }

    const auto now = NowUnixSeconds();
    Principal owner{
        .id = NewPrincipalId(),
        .kind = PrincipalKind::Owner,
        .status = PrincipalStatus::Active,
        .display_name = display_name,
        .created_unix_seconds = now,
        .updated_unix_seconds = now};

    auto inserted = store_.InsertPrincipal(owner);
    if (!inserted.Ok())
    {
        result.error = inserted.error;
        return result;
    }

    auto cred = store_.SetCredentialVerifier(
        owner.id,
        CredentialType::LocalSecret,
        HashSecret(secret));
    if (!cred.Ok())
    {
        result.error = cred.error;
        return result;
    }

    static_cast<void>(store_.AssignRole(owner.id, kRoleOwner));
    result.value = owner.id;
    return result;
}

Result<Session> IdentityService::RequireActiveOwnerSession(
    const SessionId& session_id) const
{
    auto session = ValidateSession(session_id);
    if (!session.Ok())
    {
        return session;
    }
    auto principal = store_.GetPrincipal(session.value.principal_id);
    if (!principal.Ok())
    {
        Result<Session> result;
        result.error = principal.error;
        return result;
    }
    if (principal.value.kind != PrincipalKind::Owner)
    {
        Result<Session> result;
        result.error = IdentityError::Unauthorized;
        return result;
    }
    return session;
}

Result<PrincipalId> IdentityService::CreateOperator(
    const SessionId& owner_session,
    const std::string& display_name,
    const std::string& secret)
{
    Result<PrincipalId> result;
    auto gate = RequireActiveOwnerSession(owner_session);
    if (!gate.Ok())
    {
        result.error = gate.error;
        return result;
    }
    if (display_name.empty() || secret.empty())
    {
        result.error = IdentityError::InvalidArgument;
        return result;
    }

    const auto now = NowUnixSeconds();
    Principal op{
        .id = NewPrincipalId(),
        .kind = PrincipalKind::Operator,
        .status = PrincipalStatus::Active,
        .display_name = display_name,
        .created_unix_seconds = now,
        .updated_unix_seconds = now};

    auto inserted = store_.InsertPrincipal(op);
    if (!inserted.Ok())
    {
        result.error = inserted.error;
        return result;
    }
    auto cred = store_.SetCredentialVerifier(
        op.id,
        CredentialType::LocalSecret,
        HashSecret(secret));
    if (!cred.Ok())
    {
        result.error = cred.error;
        return result;
    }
    static_cast<void>(store_.AssignRole(op.id, kRoleOperator));
    result.value = op.id;
    return result;
}

Result<Principal> IdentityService::GetPrincipal(const PrincipalId& id) const
{
    return store_.GetPrincipal(id);
}

Result<std::vector<Principal>> IdentityService::ListPrincipals(
    const SessionId& requester_session) const
{
    Result<std::vector<Principal>> result;
    auto session = ValidateSession(requester_session);
    if (!session.Ok())
    {
        result.error = session.error;
        return result;
    }
    return store_.ListPrincipals();
}

Result<void> IdentityService::SetPrincipalStatus(
    const SessionId& owner_session,
    const PrincipalId& id,
    PrincipalStatus status)
{
    Result<void> result;
    auto gate = RequireActiveOwnerSession(owner_session);
    if (!gate.Ok())
    {
        result.error = gate.error;
        return result;
    }
    auto updated = store_.UpdatePrincipalStatus(id, status);
    if (!updated.Ok())
    {
        return updated;
    }
    if (status == PrincipalStatus::Suspended ||
        status == PrincipalStatus::Revoked)
    {
        static_cast<void>(store_.RevokeAllSessions(id));
    }
    return result;
}

Result<void> IdentityService::AssignRole(
    const SessionId& owner_session,
    const PrincipalId& id,
    const std::string& role)
{
    Result<void> result;
    auto gate = RequireActiveOwnerSession(owner_session);
    if (!gate.Ok())
    {
        result.error = gate.error;
        return result;
    }
    return store_.AssignRole(id, role);
}

Result<void> IdentityService::RevokeRole(
    const SessionId& owner_session,
    const PrincipalId& id,
    const std::string& role)
{
    Result<void> result;
    auto gate = RequireActiveOwnerSession(owner_session);
    if (!gate.Ok())
    {
        result.error = gate.error;
        return result;
    }
    return store_.RevokeRole(id, role);
}

Result<std::vector<std::string>> IdentityService::GetRoles(
    const PrincipalId& id) const
{
    return store_.GetRoles(id);
}

Result<Session> IdentityService::Authenticate(
    const std::string& display_name,
    const std::string& secret)
{
    Result<Session> result;
    if (display_name.empty() || secret.empty())
    {
        result.error = IdentityError::InvalidArgument;
        return result;
    }

    auto principals = store_.ListPrincipals();
    if (!principals.Ok())
    {
        result.error = principals.error;
        return result;
    }

    const Principal* match = nullptr;
    for (const auto& p : principals.value)
    {
        if (p.display_name == display_name)
        {
            match = &p;
            break;
        }
    }
    if (match == nullptr)
    {
        result.error = IdentityError::InvalidCredential;
        return result;
    }
    if (match->status == PrincipalStatus::Suspended)
    {
        result.error = IdentityError::PrincipalSuspended;
        return result;
    }
    if (match->status == PrincipalStatus::Revoked)
    {
        result.error = IdentityError::PrincipalRevoked;
        return result;
    }

    auto verifier = store_.GetCredentialVerifier(
        match->id,
        CredentialType::LocalSecret);
    if (!verifier.Ok() || !VerifySecret(verifier.value, secret))
    {
        result.error = IdentityError::InvalidCredential;
        return result;
    }

    const auto now = NowUnixSeconds();
    Session session{
        .id = NewSessionId(),
        .principal_id = match->id,
        .status = SessionStatus::Active,
        .issued_unix_seconds = now,
        .last_activity_unix_seconds = now,
        .expires_unix_seconds = now + config_.session_lifetime_seconds,
        .origin_label = "local",
        .auth_method = CredentialType::LocalSecret};

    auto inserted = store_.InsertSession(session);
    if (!inserted.Ok())
    {
        result.error = inserted.error;
        return result;
    }
    result.value = session;
    return result;
}

Result<Session> IdentityService::ValidateSession(const SessionId& session_id) const
{
    Result<Session> result;
    auto session = store_.GetSession(session_id);
    if (!session.Ok())
    {
        result.error = session.error;
        return result;
    }
    if (session.value.status == SessionStatus::Revoked)
    {
        result.error = IdentityError::SessionRevoked;
        return result;
    }
    const auto now = NowUnixSeconds();
    if (session.value.expires_unix_seconds <= now ||
        session.value.status == SessionStatus::Expired)
    {
        result.error = IdentityError::SessionExpired;
        return result;
    }
    result.value = session.value;
    return result;
}

Result<Session> IdentityService::RefreshSession(const SessionId& session_id)
{
    auto session = ValidateSession(session_id);
    if (!session.Ok())
    {
        return session;
    }
    const auto now = NowUnixSeconds();
    session.value.last_activity_unix_seconds = now;
    session.value.expires_unix_seconds =
        now + config_.session_lifetime_seconds;
    auto updated = store_.UpdateSession(session.value);
    if (!updated.Ok())
    {
        Result<Session> result;
        result.error = updated.error;
        return result;
    }
    return session;
}

Result<void> IdentityService::RevokeSession(const SessionId& session_id)
{
    Result<void> result;
    auto session = store_.GetSession(session_id);
    if (!session.Ok())
    {
        result.error = session.error;
        return result;
    }
    session.value.status = SessionStatus::Revoked;
    return store_.UpdateSession(session.value);
}

Result<std::vector<Session>> IdentityService::ListSessions(
    const PrincipalId& id) const
{
    return store_.ListSessions(id);
}


Result<void> IdentityService::ChangeCredential(
    const SessionId& session_id,
    const std::string& current_secret,
    const std::string& new_secret)
{
    Result<void> result;
    auto session = ValidateSession(session_id);
    if (!session.Ok())
    {
        result.error = session.error;
        return result;
    }
    if (new_secret.empty())
    {
        result.error = IdentityError::InvalidArgument;
        return result;
    }
    auto principal = store_.GetPrincipal(session.value.principal_id);
    if (!principal.Ok())
    {
        result.error = principal.error;
        return result;
    }
    auto verifier = store_.GetCredentialVerifier(
        principal.value.id, CredentialType::LocalSecret);
    if (!verifier.Ok() || !VerifySecret(verifier.value, current_secret))
    {
        result.error = IdentityError::InvalidCredential;
        return result;
    }
    auto set = store_.SetCredentialVerifier(
        principal.value.id,
        CredentialType::LocalSecret,
        HashSecret(new_secret));
    if (!set.Ok())
    {
        result.error = set.error;
        return result;
    }
    return store_.RevokeAllSessions(principal.value.id);
}

} // namespace sovereign::identity
