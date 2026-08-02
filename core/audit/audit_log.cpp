#include "audit_log.hpp"

#include <chrono>
#include <fstream>
#include <random>

namespace sovereign::audit
{
namespace
{
std::string Hex(const std::array<std::uint8_t, 16>& b)
{
    static constexpr char k[] = "0123456789abcdef";
    std::string o(32, '0');
    for (std::size_t i = 0; i < 16; ++i)
    {
        o[i * 2] = k[(b[i] >> 4) & 0xF];
        o[i * 2 + 1] = k[b[i] & 0xF];
    }
    return o;
}
bool Unhex(const std::string& h, std::array<std::uint8_t, 16>& b)
{
    if (h.size() != 32)
        return false;
    auto n = [](char c) {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < 16; ++i)
    {
        int hi = n(h[i * 2]), lo = n(h[i * 2 + 1]);
        if (hi < 0 || lo < 0)
            return false;
        b[i] = static_cast<std::uint8_t>((hi << 4) | lo);
    }
    return true;
}
std::string Esc(const std::string& s)
{
    std::string o;
    for (char c : s)
    {
        if (c == '|' || c == '\\' || c == '\n')
        {
            o.push_back('\\');
            o.push_back(c == '\n' ? 'n' : c);
        }
        else
            o.push_back(c);
    }
    return o;
}
std::string Unesc(const std::string& s)
{
    std::string o;
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '\\' && i + 1 < s.size())
        {
            ++i;
            o.push_back(s[i] == 'n' ? '\n' : s[i]);
        }
        else
            o.push_back(s[i]);
    }
    return o;
}
} // namespace

AuditEntryId AuditLog::NewId()
{
    AuditEntryId id{};
    std::random_device rd;
    for (auto& x : id.bytes)
        x = static_cast<std::uint8_t>(rd());
    return id;
}

std::uint64_t AuditLog::Now()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

AuditError AuditLog::Open(const std::filesystem::path& directory)
{
    std::lock_guard lock(mutex_);
    directory_ = directory;
    std::error_code ec;
    std::filesystem::create_directories(directory_, ec);
    if (ec)
        return AuditError::StorageFailure;
    open_ = true;
    return Load();
}

void AuditLog::Close() noexcept
{
    std::lock_guard lock(mutex_);
    open_ = false;
}

AuditError AuditLog::Load()
{
    entries_.clear();
    std::ifstream in(directory_ / "audit.dat");
    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty())
            continue;
        std::vector<std::string> p;
        std::string cur;
        for (std::size_t i = 0; i < line.size(); ++i)
        {
            if (line[i] == '\\' && i + 1 < line.size())
            {
                cur.push_back(line[i++]);
                cur.push_back(line[i]);
            }
            else if (line[i] == '|')
            {
                p.push_back(cur);
                cur.clear();
            }
            else
                cur.push_back(line[i]);
        }
        p.push_back(cur);
        if (p.size() < 7)
            continue;
        AuditEntry e;
        if (!Unhex(p[0], e.id.bytes))
            continue;
        e.unix_seconds = std::stoull(p[1]);
        e.category = Unesc(p[2]);
        e.action = Unesc(p[3]);
        Unhex(p[4], e.actor.bytes);
        e.subject = Unesc(p[5]);
        e.detail = Unesc(p[6]);
        entries_.push_back(e);
    }
    return AuditError::Ok;
}

AuditError AuditLog::AppendLine(const AuditEntry& e) const
{
    std::ofstream out(directory_ / "audit.dat", std::ios::app);
    if (!out)
        return AuditError::StorageFailure;
    out << Hex(e.id.bytes) << '|' << e.unix_seconds << '|' << Esc(e.category)
        << '|' << Esc(e.action) << '|' << Hex(e.actor.bytes) << '|'
        << Esc(e.subject) << '|' << Esc(e.detail) << '\n';
    return AuditError::Ok;
}

Result<AuditEntryId> AuditLog::Append(const AuditEntry& entry)
{
    std::lock_guard lock(mutex_);
    Result<AuditEntryId> result;
    if (!open_ || entry.category.empty() || entry.action.empty())
    {
        result.error = AuditError::InvalidArgument;
        return result;
    }
    AuditEntry e = entry;
    bool nil = true;
    for (auto b : e.id.bytes)
        if (b)
        {
            nil = false;
            break;
        }
    if (nil)
        e.id = NewId();
    if (e.unix_seconds == 0)
        e.unix_seconds = Now();
    if (AppendLine(e) != AuditError::Ok)
    {
        result.error = AuditError::StorageFailure;
        return result;
    }
    entries_.push_back(e);
    result.value = e.id;
    return result;
}

Result<std::vector<AuditEntry>> AuditLog::List(std::size_t limit) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<AuditEntry>> result;
    const std::size_t n = entries_.size();
    const std::size_t start = n > limit ? n - limit : 0;
    for (std::size_t i = start; i < n; ++i)
        result.value.push_back(entries_[i]);
    return result;
}

Result<std::vector<AuditEntry>> AuditLog::ListByCategory(
    const std::string& category,
    std::size_t limit) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<AuditEntry>> result;
    for (auto it = entries_.rbegin();
         it != entries_.rend() && result.value.size() < limit;
         ++it)
    {
        if (it->category == category)
            result.value.push_back(*it);
    }
    return result;
}

} // namespace sovereign::audit
