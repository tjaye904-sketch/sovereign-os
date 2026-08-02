#include "identity_store_memory.hpp"

#include <algorithm>
#include <sstream>

namespace sovereign::identity
{
namespace
{

std::string VerifierKey(const PrincipalId& id, CredentialType type)
{
    std::ostringstream oss;
    for (auto b : id.bytes)
    {
        oss.width(2);
        oss.fill('0');
        oss << std::hex << static_cast<int>(b);
    }
    oss << ":" << static_cast<int>(type);
    return oss.str();
}

} // namespace

std::string MemoryIdentityStore::IdKey(const PrincipalId& id)
{
    std::string key;
    key.resize(id.bytes.size());
    for (std::size_t i = 0; i < id.bytes.size(); ++i)
    {
        key[i] = static_cast<char>(id.bytes[i]);
    }
    return key;
}

std::string MemoryIdentityStore::IdKey(const SessionId& id)
{
    std::string key;
    key.resize(id.bytes.size());
    for (std::size_t i = 0; i < id.bytes.size(); ++i)
    {
        key[i] = static_cast<char>(id.bytes[i]);
    }
    return key;
}

IdentityError MemoryIdentityStore::Open(const std::filesystem::path& directory)
{
    std::lock_guard lock(mutex_);
    directory_ = directory;
    open_ = true;
    return IdentityError::Ok;
}

void MemoryIdentityStore::Close() noexcept
{
    std::lock_guard lock(mutex_);
    open_ = false;
}

Result<PrincipalId> MemoryIdentityStore::InsertPrincipal(const Principal& principal)
{
    std::lock_guard lock(mutex_);
    Result<PrincipalId> result;
    if (!open_)
    {
        result.error = IdentityError::StorageFailure;
        return result;
    }
    if (IsNil(principal.id))
    {
        result.error = IdentityError::InvalidArgument;
        return result;
    }
    const auto key = IdKey(principal.id);
    if (principals_.contains(key))
    {
        result.error = IdentityError::AlreadyExists;
        return result;
    }
    principals_.emplace(key, principal);
    result.value = principal.id;
    return result;
}

Result<Principal> MemoryIdentityStore::GetPrincipal(const PrincipalId& id) const
{
    std::lock_guard lock(mutex_);
    Result<Principal> result;
    const auto it = principals_.find(IdKey(id));
    if (it == principals_.end())
    {
        result.error = IdentityError::NotFound;
        return result;
    }
    result.value = it->second;
    return result;
}

Result<std::vector<Principal>> MemoryIdentityStore::ListPrincipals() const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<Principal>> result;
    result.value.reserve(principals_.size());
    for (const auto& [_, p] : principals_)
    {
        result.value.push_back(p);
    }
    return result;
}

Result<void> MemoryIdentityStore::UpdatePrincipalStatus(
    const PrincipalId& id,
    PrincipalStatus status)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    auto it = principals_.find(IdKey(id));
    if (it == principals_.end())
    {
        result.error = IdentityError::NotFound;
        return result;
    }
    it->second.status = status;
    return result;
}

Result<void> MemoryIdentityStore::SetCredentialVerifier(
    const PrincipalId& id,
    CredentialType type,
    const std::string& verifier)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    if (!principals_.contains(IdKey(id)))
    {
        result.error = IdentityError::NotFound;
        return result;
    }
    verifiers_[VerifierKey(id, type)] = verifier;
    return result;
}

Result<std::string> MemoryIdentityStore::GetCredentialVerifier(
    const PrincipalId& id,
    CredentialType type) const
{
    std::lock_guard lock(mutex_);
    Result<std::string> result;
    const auto it = verifiers_.find(VerifierKey(id, type));
    if (it == verifiers_.end())
    {
        result.error = IdentityError::NotFound;
        return result;
    }
    result.value = it->second;
    return result;
}

Result<void> MemoryIdentityStore::AssignRole(
    const PrincipalId& id,
    const std::string& role)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    if (!principals_.contains(IdKey(id)))
    {
        result.error = IdentityError::NotFound;
        return result;
    }
    auto& list = roles_[IdKey(id)];
    if (std::find(list.begin(), list.end(), role) == list.end())
    {
        list.push_back(role);
    }
    return result;
}

Result<void> MemoryIdentityStore::RevokeRole(
    const PrincipalId& id,
    const std::string& role)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    auto it = roles_.find(IdKey(id));
    if (it == roles_.end())
    {
        return result;
    }
    auto& list = it->second;
    list.erase(std::remove(list.begin(), list.end(), role), list.end());
    return result;
}

Result<std::vector<std::string>> MemoryIdentityStore::GetRoles(
    const PrincipalId& id) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<std::string>> result;
    const auto it = roles_.find(IdKey(id));
    if (it != roles_.end())
    {
        result.value = it->second;
    }
    return result;
}

Result<SessionId> MemoryIdentityStore::InsertSession(const Session& session)
{
    std::lock_guard lock(mutex_);
    Result<SessionId> result;
    if (IsNil(session.id))
    {
        result.error = IdentityError::InvalidArgument;
        return result;
    }
    sessions_[IdKey(session.id)] = session;
    result.value = session.id;
    return result;
}

Result<Session> MemoryIdentityStore::GetSession(const SessionId& id) const
{
    std::lock_guard lock(mutex_);
    Result<Session> result;
    const auto it = sessions_.find(IdKey(id));
    if (it == sessions_.end())
    {
        result.error = IdentityError::NotFound;
        return result;
    }
    result.value = it->second;
    return result;
}

Result<std::vector<Session>> MemoryIdentityStore::ListSessions(
    const PrincipalId& principal_id) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<Session>> result;
    for (const auto& [_, session] : sessions_)
    {
        if (session.principal_id == principal_id)
        {
            result.value.push_back(session);
        }
    }
    return result;
}

Result<void> MemoryIdentityStore::UpdateSession(const Session& session)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    auto it = sessions_.find(IdKey(session.id));
    if (it == sessions_.end())
    {
        result.error = IdentityError::NotFound;
        return result;
    }
    it->second = session;
    return result;
}

Result<void> MemoryIdentityStore::RevokeAllSessions(const PrincipalId& principal_id)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& [_, session] : sessions_)
    {
        if (session.principal_id == principal_id &&
            session.status == SessionStatus::Active)
        {
            session.status = SessionStatus::Revoked;
        }
    }
    return result;
}

Result<std::optional<Principal>> MemoryIdentityStore::FindOwner() const
{
    std::lock_guard lock(mutex_);
    Result<std::optional<Principal>> result;
    for (const auto& [_, p] : principals_)
    {
        if (p.kind == PrincipalKind::Owner)
        {
            result.value = p;
            return result;
        }
    }
    result.value = std::nullopt;
    return result;
}

} // namespace sovereign::identity
