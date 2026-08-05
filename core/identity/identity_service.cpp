#include "identity_service.hpp"
#include "identity_totp.hpp"
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


Result<PrincipalId> IdentityService::CreateServicePrincipal(
    const SessionId& owner_session,
    const std::string& display_name)
{
    Result<PrincipalId> result;
    auto gate = RequireActiveOwnerSession(owner_session);
    if (!gate.Ok())
    {
        result.error = gate.error;
        return result;
    }
    if (display_name.empty())
    {
        result.error = IdentityError::InvalidArgument;
        return result;
    }
    const auto now = NowUnixSeconds();
    Principal svc{
        .id = NewPrincipalId(),
        .kind = PrincipalKind::Service,
        .status = PrincipalStatus::Active,
        .display_name = display_name,
        .created_unix_seconds = now,
        .updated_unix_seconds = now};
    auto inserted = store_.InsertPrincipal(svc);
    if (!inserted.Ok())
    {
        result.error = inserted.error;
        return result;
    }
    static_cast<void>(store_.AssignRole(svc.id, kRoleService));
    result.value = svc.id;
    return result;
}

Result<PrincipalId> IdentityService::CreateMachinePrincipal(
    const SessionId& owner_session,
    const std::string& display_name)
{
    Result<PrincipalId> result;
    auto gate = RequireActiveOwnerSession(owner_session);
    if (!gate.Ok())
    {
        result.error = gate.error;
        return result;
    }
    if (display_name.empty())
    {
        result.error = IdentityError::InvalidArgument;
        return result;
    }
    const auto now = NowUnixSeconds();
    Principal dev{
        .id = NewPrincipalId(),
        .kind = PrincipalKind::Device,
        .status = PrincipalStatus::Active,
        .display_name = display_name,
        .created_unix_seconds = now,
        .updated_unix_seconds = now};
    auto inserted = store_.InsertPrincipal(dev);
    if (!inserted.Ok())
    {
        result.error = inserted.error;
        return result;
    }
    // Machines carry operator-class access only when roles assigned explicitly
    result.value = dev.id;
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

    if (config_.require_mfa_on_login)
    {
        auto st = GetMfaStatus(match->id);
        if (st.Ok() && st.value.totp_enrolled)
        {
            result.error = IdentityError::MfaRequired;
            return result;
        }
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


Result<IdentityService::MfaStatus> IdentityService::GetMfaStatus(
    const PrincipalId& id) const
{
    Result<MfaStatus> result;
    auto v = store_.GetCredentialVerifier(id, CredentialType::TotpSecret);
    result.value.totp_enrolled =
        v.Ok() && !v.value.empty() && v.value.rfind("pending:", 0) != 0;
    result.value.step_up_required_for_execute = result.value.totp_enrolled;
    return result;
}

Result<std::string> IdentityService::BeginTotpEnrollment(const SessionId& session)
{
    Result<std::string> result;
    auto s = ValidateSession(session);
    if (!s.Ok())
    {
        result.error = s.error;
        return result;
    }
    auto secret = totp::RandomSecret(20);
    auto b32 = totp::Base32Encode(secret);
    // Stage pending secret as TotpSecret only after confirm — store under Local path:
    // use a reserved verifier key via TotpSecret with "pending:" prefix
    auto set = store_.SetCredentialVerifier(
        s.value.principal_id, CredentialType::TotpSecret, std::string("pending:") + b32);
    if (!set.Ok())
    {
        result.error = set.error;
        return result;
    }
    auto p = store_.GetPrincipal(s.value.principal_id);
    std::string name = p.Ok() ? p.value.display_name : "user";
    result.value = totp::ProvisioningUri(b32, name);
    return result;
}

Result<void> IdentityService::ConfirmTotpEnrollment(
    const SessionId& session, const std::string& code)
{
    Result<void> result;
    auto s = ValidateSession(session);
    if (!s.Ok())
    {
        result.error = s.error;
        return result;
    }
    auto v = store_.GetCredentialVerifier(s.value.principal_id, CredentialType::TotpSecret);
    if (!v.Ok() || v.value.rfind("pending:", 0) != 0)
    {
        result.error = IdentityError::MfaNotEnrolled;
        return result;
    }
    const std::string b32 = v.value.substr(8);
    const auto now = static_cast<std::int64_t>(NowUnixSeconds());
    if (!totp::VerifyCode(b32, code, now))
    {
        result.error = IdentityError::MfaInvalid;
        return result;
    }
    auto set = store_.SetCredentialVerifier(
        s.value.principal_id, CredentialType::TotpSecret, b32);
    if (!set.Ok())
    {
        result.error = set.error;
        return result;
    }
    return result;
}

Result<Session> IdentityService::AuthenticateWithMfa(
    const std::string& display_name,
    const std::string& secret,
    const std::string& totp_code)
{
    // Bypass login MFA gate so password can succeed; TOTP checked below.
    const bool prev = config_.require_mfa_on_login;
    config_.require_mfa_on_login = false;
    auto base = Authenticate(display_name, secret);
    config_.require_mfa_on_login = prev;
    if (!base.Ok())
        return base;
    auto st = GetMfaStatus(base.value.principal_id);
    if (!st.Ok() || !st.value.totp_enrolled)
        return base;
    auto v = store_.GetCredentialVerifier(
        base.value.principal_id, CredentialType::TotpSecret);
    if (!v.Ok() || v.value.empty() || v.value.rfind("pending:", 0) == 0)
        return base;
    const auto now = static_cast<std::int64_t>(NowUnixSeconds());
    if (!totp::VerifyCode(v.value, totp_code, now))
    {
        Result<Session> fail;
        fail.error = IdentityError::MfaInvalid;
        static_cast<void>(RevokeSession(base.value.id));
        return fail;
    }
    return base;
}


Result<std::vector<std::string>> IdentityService::GenerateRecoveryCodes(
    const SessionId& session)
{
    Result<std::vector<std::string>> result;
    auto s = ValidateSession(session);
    if (!s.Ok())
    {
        result.error = s.error;
        return result;
    }
    std::vector<std::string> plain;
    // Newline-separated Argon2 verifiers (hashes contain commas; never split on ',')
    std::string store_blob;
    for (int i = 0; i < 8; ++i)
    {
        auto raw = totp::RandomSecret(5);
        std::string code = totp::Base32Encode(raw).substr(0, 10);
        plain.push_back(code);
        auto hashed = HashSecret(code);
        if (!store_blob.empty())
            store_blob.push_back('\n');
        store_blob += hashed;
    }
    auto set = store_.SetCredentialVerifier(
        s.value.principal_id, CredentialType::RecoveryCodes, store_blob);
    if (!set.Ok())
    {
        result.error = set.error;
        return result;
    }
    result.value = std::move(plain);
    return result;
}

Result<void> IdentityService::ConsumeRecoveryCode(
    const PrincipalId& id, const std::string& code)
{
    Result<void> result;
    auto v = store_.GetCredentialVerifier(id, CredentialType::RecoveryCodes);
    if (!v.Ok() || v.value.empty())
    {
        result.error = IdentityError::MfaNotEnrolled;
        return result;
    }
    // Argon2 salts differ per hash — must VerifySecret, not re-hash and compare.
    std::string rest;
    bool found = false;
    std::size_t start = 0;
    while (start <= v.value.size())
    {
        auto nl = v.value.find('\n', start);
        std::string part = v.value.substr(
            start, nl == std::string::npos ? std::string::npos : nl - start);
        if (!part.empty())
        {
            if (!found && VerifySecret(part, code))
                found = true;
            else
            {
                if (!rest.empty())
                    rest.push_back('\n');
                rest += part;
            }
        }
        if (nl == std::string::npos)
            break;
        start = nl + 1;
    }
    if (!found)
    {
        result.error = IdentityError::MfaInvalid;
        return result;
    }
    auto set = store_.SetCredentialVerifier(id, CredentialType::RecoveryCodes, rest);
    if (!set.Ok())
    {
        result.error = set.error;
        return result;
    }
    return result;
}

Result<void> IdentityService::VerifyStepUp(
    const SessionId& session, const std::string& totp_or_recovery)
{
    Result<void> result;
    auto s = ValidateSession(session);
    if (!s.Ok())
    {
        result.error = s.error;
        return result;
    }
    auto st = GetMfaStatus(s.value.principal_id);
    if (!st.Ok() || !st.value.step_up_required_for_execute)
        return result; // no step-up required
    auto v = store_.GetCredentialVerifier(s.value.principal_id, CredentialType::TotpSecret);
    if (v.Ok() && !v.value.empty() && v.value.rfind("pending:", 0) != 0)
    {
        const auto now = static_cast<std::int64_t>(NowUnixSeconds());
        if (totp::VerifyCode(v.value, totp_or_recovery, now))
            return result;
    }
    auto cons = ConsumeRecoveryCode(s.value.principal_id, totp_or_recovery);
    if (cons.Ok())
        return result;
    result.error = IdentityError::MfaInvalid;
    return result;
}

} // namespace sovereign::identity
