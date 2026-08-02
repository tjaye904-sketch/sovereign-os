#include "identity_store_file.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace sovereign::identity
{
namespace
{

std::string HexEncode(const std::array<std::uint8_t, 16>& bytes)
{
    static constexpr char kHex[] = "0123456789abcdef";
    std::string out;
    out.resize(32);
    for (std::size_t i = 0; i < 16; ++i)
    {
        out[i * 2] = kHex[(bytes[i] >> 4) & 0xF];
        out[i * 2 + 1] = kHex[bytes[i] & 0xF];
    }
    return out;
}

bool HexDecode(const std::string& hex, std::array<std::uint8_t, 16>& bytes)
{
    if (hex.size() != 32)
    {
        return false;
    }
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < 16; ++i)
    {
        const int hi = nibble(hex[i * 2]);
        const int lo = nibble(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0)
        {
            return false;
        }
        bytes[i] = static_cast<std::uint8_t>((hi << 4) | lo);
    }
    return true;
}

std::string Escape(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s)
    {
        if (c == '\\' || c == '|' || c == '\n' || c == '\r')
        {
            out.push_back('\\');
            if (c == '\n') out.push_back('n');
            else if (c == '\r') out.push_back('r');
            else out.push_back(c);
        }
        else
        {
            out.push_back(c);
        }
    }
    return out;
}

std::string Unescape(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '\\' && i + 1 < s.size())
        {
            ++i;
            if (s[i] == 'n') out.push_back('\n');
            else if (s[i] == 'r') out.push_back('\r');
            else out.push_back(s[i]);
        }
        else
        {
            out.push_back(s[i]);
        }
    }
    return out;
}

std::vector<std::string> SplitPipe(const std::string& line)
{
    std::vector<std::string> parts;
    std::string cur;
    for (std::size_t i = 0; i < line.size(); ++i)
    {
        if (line[i] == '\\' && i + 1 < line.size())
        {
            cur.push_back(line[i]);
            cur.push_back(line[++i]);
        }
        else if (line[i] == '|')
        {
            parts.push_back(cur);
            cur.clear();
        }
        else
        {
            cur.push_back(line[i]);
        }
    }
    parts.push_back(cur);
    return parts;
}

} // namespace

IdentityError FileIdentityStore::Open(const std::filesystem::path& directory)
{
    std::lock_guard lock(mutex_);
    directory_ = directory;
    std::error_code ec;
    std::filesystem::create_directories(directory_, ec);
    if (ec)
    {
        return IdentityError::StorageFailure;
    }
    open_ = true;
    return LoadAll();
}

void FileIdentityStore::Close() noexcept
{
    std::lock_guard lock(mutex_);
    if (open_)
    {
        static_cast<void>(SaveAll());
    }
    open_ = false;
}

IdentityError FileIdentityStore::LoadAll()
{
    principals_.clear();
    verifiers_.clear();
    roles_.clear();
    sessions_.clear();

    {
        std::ifstream in(directory_ / "principals.dat");
        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty()) continue;
            auto p = SplitPipe(line);
            if (p.size() < 6) continue;
            Principal pr;
            if (!HexDecode(p[0], pr.id.bytes)) continue;
            pr.kind = static_cast<PrincipalKind>(std::stoi(p[1]));
            pr.status = static_cast<PrincipalStatus>(std::stoi(p[2]));
            pr.display_name = Unescape(p[3]);
            pr.created_unix_seconds = std::stoull(p[4]);
            pr.updated_unix_seconds = std::stoull(p[5]);
            principals_.push_back(pr);
        }
    }
    {
        std::ifstream in(directory_ / "verifiers.dat");
        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty()) continue;
            auto p = SplitPipe(line);
            if (p.size() < 3) continue;
            VerifierRecord vr;
            if (!HexDecode(p[0], vr.id.bytes)) continue;
            vr.type = static_cast<CredentialType>(std::stoi(p[1]));
            vr.verifier = Unescape(p[2]);
            verifiers_.push_back(vr);
        }
    }
    {
        std::ifstream in(directory_ / "roles.dat");
        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty()) continue;
            auto p = SplitPipe(line);
            if (p.size() < 2) continue;
            RoleRecord rr;
            if (!HexDecode(p[0], rr.id.bytes)) continue;
            rr.role = Unescape(p[1]);
            roles_.push_back(rr);
        }
    }
    {
        std::ifstream in(directory_ / "sessions.dat");
        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty()) continue;
            auto p = SplitPipe(line);
            if (p.size() < 9) continue;
            Session s;
            if (!HexDecode(p[0], s.id.bytes)) continue;
            if (!HexDecode(p[1], s.principal_id.bytes)) continue;
            s.status = static_cast<SessionStatus>(std::stoi(p[2]));
            s.issued_unix_seconds = std::stoull(p[3]);
            s.last_activity_unix_seconds = std::stoull(p[4]);
            s.expires_unix_seconds = std::stoull(p[5]);
            s.origin_label = Unescape(p[6]);
            s.auth_method = static_cast<CredentialType>(std::stoi(p[7]));
            sessions_.push_back(s);
        }
    }
    return IdentityError::Ok;
}

IdentityError FileIdentityStore::SaveAll() const
{
    {
        std::ofstream out(directory_ / "principals.dat", std::ios::trunc);
        if (!out) return IdentityError::StorageFailure;
        for (const auto& pr : principals_)
        {
            out << HexEncode(pr.id.bytes) << '|'
                << static_cast<int>(pr.kind) << '|'
                << static_cast<int>(pr.status) << '|'
                << Escape(pr.display_name) << '|'
                << pr.created_unix_seconds << '|'
                << pr.updated_unix_seconds << '\n';
        }
    }
    {
        std::ofstream out(directory_ / "verifiers.dat", std::ios::trunc);
        if (!out) return IdentityError::StorageFailure;
        for (const auto& vr : verifiers_)
        {
            out << HexEncode(vr.id.bytes) << '|'
                << static_cast<int>(vr.type) << '|'
                << Escape(vr.verifier) << '\n';
        }
    }
    {
        std::ofstream out(directory_ / "roles.dat", std::ios::trunc);
        if (!out) return IdentityError::StorageFailure;
        for (const auto& rr : roles_)
        {
            out << HexEncode(rr.id.bytes) << '|'
                << Escape(rr.role) << '\n';
        }
    }
    {
        std::ofstream out(directory_ / "sessions.dat", std::ios::trunc);
        if (!out) return IdentityError::StorageFailure;
        for (const auto& s : sessions_)
        {
            out << HexEncode(s.id.bytes) << '|'
                << HexEncode(s.principal_id.bytes) << '|'
                << static_cast<int>(s.status) << '|'
                << s.issued_unix_seconds << '|'
                << s.last_activity_unix_seconds << '|'
                << s.expires_unix_seconds << '|'
                << Escape(s.origin_label) << '|'
                << static_cast<int>(s.auth_method) << "|\n";
        }
    }
    return IdentityError::Ok;
}

Result<PrincipalId> FileIdentityStore::InsertPrincipal(const Principal& principal)
{
    std::lock_guard lock(mutex_);
    Result<PrincipalId> result;
    if (!open_ || IsNil(principal.id))
    {
        result.error = IdentityError::InvalidArgument;
        return result;
    }
    for (const auto& p : principals_)
    {
        if (p.id == principal.id)
        {
            result.error = IdentityError::AlreadyExists;
            return result;
        }
    }
    principals_.push_back(principal);
    if (SaveAll() != IdentityError::Ok)
    {
        principals_.pop_back();
        result.error = IdentityError::StorageFailure;
        return result;
    }
    result.value = principal.id;
    return result;
}

Result<Principal> FileIdentityStore::GetPrincipal(const PrincipalId& id) const
{
    std::lock_guard lock(mutex_);
    Result<Principal> result;
    for (const auto& p : principals_)
    {
        if (p.id == id)
        {
            result.value = p;
            return result;
        }
    }
    result.error = IdentityError::NotFound;
    return result;
}

Result<std::vector<Principal>> FileIdentityStore::ListPrincipals() const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<Principal>> result;
    result.value = principals_;
    return result;
}

Result<void> FileIdentityStore::UpdatePrincipalStatus(
    const PrincipalId& id,
    PrincipalStatus status)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& p : principals_)
    {
        if (p.id == id)
        {
            p.status = status;
            result.error = SaveAll();
            return result;
        }
    }
    result.error = IdentityError::NotFound;
    return result;
}

Result<void> FileIdentityStore::SetCredentialVerifier(
    const PrincipalId& id,
    CredentialType type,
    const std::string& verifier)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    bool found = false;
    for (const auto& p : principals_)
    {
        if (p.id == id)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        result.error = IdentityError::NotFound;
        return result;
    }
    for (auto& vr : verifiers_)
    {
        if (vr.id == id && vr.type == type)
        {
            vr.verifier = verifier;
            result.error = SaveAll();
            return result;
        }
    }
    verifiers_.push_back(VerifierRecord{id, type, verifier});
    result.error = SaveAll();
    return result;
}

Result<std::string> FileIdentityStore::GetCredentialVerifier(
    const PrincipalId& id,
    CredentialType type) const
{
    std::lock_guard lock(mutex_);
    Result<std::string> result;
    for (const auto& vr : verifiers_)
    {
        if (vr.id == id && vr.type == type)
        {
            result.value = vr.verifier;
            return result;
        }
    }
    result.error = IdentityError::NotFound;
    return result;
}

Result<void> FileIdentityStore::AssignRole(
    const PrincipalId& id,
    const std::string& role)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    bool found = false;
    for (const auto& p : principals_)
    {
        if (p.id == id)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        result.error = IdentityError::NotFound;
        return result;
    }
    for (const auto& rr : roles_)
    {
        if (rr.id == id && rr.role == role)
        {
            return result;
        }
    }
    roles_.push_back(RoleRecord{id, role});
    result.error = SaveAll();
    return result;
}

Result<void> FileIdentityStore::RevokeRole(
    const PrincipalId& id,
    const std::string& role)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    roles_.erase(
        std::remove_if(
            roles_.begin(),
            roles_.end(),
            [&](const RoleRecord& rr) {
                return rr.id == id && rr.role == role;
            }),
        roles_.end());
    result.error = SaveAll();
    return result;
}

Result<std::vector<std::string>> FileIdentityStore::GetRoles(
    const PrincipalId& id) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<std::string>> result;
    for (const auto& rr : roles_)
    {
        if (rr.id == id)
        {
            result.value.push_back(rr.role);
        }
    }
    return result;
}

Result<SessionId> FileIdentityStore::InsertSession(const Session& session)
{
    std::lock_guard lock(mutex_);
    Result<SessionId> result;
    if (IsNil(session.id))
    {
        result.error = IdentityError::InvalidArgument;
        return result;
    }
    sessions_.push_back(session);
    if (SaveAll() != IdentityError::Ok)
    {
        sessions_.pop_back();
        result.error = IdentityError::StorageFailure;
        return result;
    }
    result.value = session.id;
    return result;
}

Result<Session> FileIdentityStore::GetSession(const SessionId& id) const
{
    std::lock_guard lock(mutex_);
    Result<Session> result;
    for (const auto& s : sessions_)
    {
        if (s.id == id)
        {
            result.value = s;
            return result;
        }
    }
    result.error = IdentityError::NotFound;
    return result;
}

Result<std::vector<Session>> FileIdentityStore::ListSessions(
    const PrincipalId& principal_id) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<Session>> result;
    for (const auto& s : sessions_)
    {
        if (s.principal_id == principal_id)
        {
            result.value.push_back(s);
        }
    }
    return result;
}

Result<void> FileIdentityStore::UpdateSession(const Session& session)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& s : sessions_)
    {
        if (s.id == session.id)
        {
            s = session;
            result.error = SaveAll();
            return result;
        }
    }
    result.error = IdentityError::NotFound;
    return result;
}

Result<void> FileIdentityStore::RevokeAllSessions(const PrincipalId& principal_id)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& s : sessions_)
    {
        if (s.principal_id == principal_id &&
            s.status == SessionStatus::Active)
        {
            s.status = SessionStatus::Revoked;
        }
    }
    result.error = SaveAll();
    return result;
}

Result<std::optional<Principal>> FileIdentityStore::FindOwner() const
{
    std::lock_guard lock(mutex_);
    Result<std::optional<Principal>> result;
    for (const auto& p : principals_)
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
