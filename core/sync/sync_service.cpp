#include "sync_service.hpp"

#include <chrono>
#include <fstream>
#include <random>

namespace sovereign::sync
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

SyncEntryId SyncService::NewId()
{
    SyncEntryId id{};
    std::random_device rd;
    for (auto& x : id.bytes)
        x = static_cast<std::uint8_t>(rd());
    return id;
}

std::uint64_t SyncService::Now()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

SyncError SyncService::Open(const std::filesystem::path& directory)
{
    std::lock_guard lock(mutex_);
    directory_ = directory;
    std::error_code ec;
    std::filesystem::create_directories(directory_, ec);
    if (ec)
        return SyncError::StorageFailure;
    open_ = true;
    return Load();
}

void SyncService::Close() noexcept
{
    std::lock_guard lock(mutex_);
    if (open_)
        static_cast<void>(Save());
    open_ = false;
}

SyncError SyncService::Load()
{
    entries_.clear();
    std::ifstream in(directory_ / "sync_journal.dat");
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
        if (p.size() < 6)
            continue;
        SyncEntry e;
        if (!Unhex(p[0], e.id.bytes))
            continue;
        Unhex(p[1], e.actor.bytes);
        e.direction = static_cast<SyncDirection>(std::stoi(p[2]));
        e.channel = Unesc(p[3]);
        e.payload_ref = Unesc(p[4]);
        e.created_unix_seconds = std::stoull(p[5]);
        entries_.push_back(e);
    }
    return SyncError::Ok;
}

SyncError SyncService::Save() const
{
    std::ofstream out(directory_ / "sync_journal.dat", std::ios::trunc);
    if (!out)
        return SyncError::StorageFailure;
    for (const auto& e : entries_)
    {
        out << Hex(e.id.bytes) << '|' << Hex(e.actor.bytes) << '|'
            << static_cast<int>(e.direction) << '|' << Esc(e.channel) << '|'
            << Esc(e.payload_ref) << '|' << e.created_unix_seconds << '\n';
    }
    return SyncError::Ok;
}

Result<SyncEntryId> SyncService::Append(const SyncEntry& entry)
{
    Result<SyncEntryId> result;
    if (!open_)
    {
        result.error = SyncError::StorageFailure;
        return result;
    }
    entries_.push_back(entry);
    if (Save() != SyncError::Ok)
    {
        entries_.pop_back();
        result.error = SyncError::StorageFailure;
        return result;
    }
    result.value = entry.id;
    return result;
}

Result<SyncEntryId> SyncService::RecordExport(
    const PrincipalId& actor,
    const std::string& channel,
    const std::string& payload_ref)
{
    std::lock_guard lock(mutex_);
    Result<SyncEntryId> result;
    if (channel.empty() || payload_ref.empty())
    {
        result.error = SyncError::InvalidArgument;
        return result;
    }
    SyncEntry e;
    e.id = NewId();
    e.actor = actor;
    e.direction = SyncDirection::Export;
    e.channel = channel;
    e.payload_ref = payload_ref;
    e.created_unix_seconds = Now();
    return Append(e);
}

Result<SyncEntryId> SyncService::RecordImport(
    const PrincipalId& actor,
    const std::string& channel,
    const std::string& payload_ref)
{
    std::lock_guard lock(mutex_);
    Result<SyncEntryId> result;
    if (channel.empty() || payload_ref.empty())
    {
        result.error = SyncError::InvalidArgument;
        return result;
    }
    SyncEntry e;
    e.id = NewId();
    e.actor = actor;
    e.direction = SyncDirection::Import;
    e.channel = channel;
    e.payload_ref = payload_ref;
    e.created_unix_seconds = Now();
    return Append(e);
}

Result<std::vector<SyncEntry>> SyncService::List() const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<SyncEntry>> result;
    result.value = entries_;
    return result;
}

} // namespace sovereign::sync
