#include "workspace_service.hpp"

#include <chrono>
#include <fstream>
#include <random>

namespace sovereign::workspace
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
    if (h.size() != 32) return false;
    auto n = [](char c) {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < 16; ++i)
    {
        int hi = n(h[i * 2]), lo = n(h[i * 2 + 1]);
        if (hi < 0 || lo < 0) return false;
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
        else o.push_back(c);
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
        else o.push_back(s[i]);
    }
    return o;
}
} // namespace

WorkspaceId WorkspaceService::NewId()
{
    WorkspaceId id{};
    std::random_device rd;
    for (auto& x : id.bytes) x = static_cast<std::uint8_t>(rd());
    return id;
}

std::uint64_t WorkspaceService::Now()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

WorkspaceError WorkspaceService::Open(const std::filesystem::path& directory)
{
    std::lock_guard lock(mutex_);
    directory_ = directory;
    std::error_code ec;
    std::filesystem::create_directories(directory_, ec);
    if (ec) return WorkspaceError::StorageFailure;
    open_ = true;
    return Load();
}

void WorkspaceService::Close() noexcept
{
    std::lock_guard lock(mutex_);
    if (open_) static_cast<void>(Save());
    open_ = false;
}

WorkspaceError WorkspaceService::Load()
{
    workspaces_.clear();
    std::ifstream in(directory_ / "workspaces.dat");
    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty()) continue;
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
            else cur.push_back(line[i]);
        }
        p.push_back(cur);
        if (p.size() < 5) continue;
        Workspace w;
        if (!Unhex(p[0], w.id.bytes)) continue;
        w.name = Unesc(p[1]);
        w.kind = Unesc(p[2]);
        w.status = static_cast<WorkspaceStatus>(std::stoi(p[3]));
        w.created_unix_seconds = std::stoull(p[4]);
        workspaces_.push_back(w);
    }
    return WorkspaceError::Ok;
}

WorkspaceError WorkspaceService::Save() const
{
    std::ofstream out(directory_ / "workspaces.dat", std::ios::trunc);
    if (!out) return WorkspaceError::StorageFailure;
    for (const auto& w : workspaces_)
    {
        out << Hex(w.id.bytes) << '|'
            << Esc(w.name) << '|'
            << Esc(w.kind) << '|'
            << static_cast<int>(w.status) << '|'
            << w.created_unix_seconds << '\n';
    }
    return WorkspaceError::Ok;
}

Result<WorkspaceId> WorkspaceService::Create(
    const std::string& name,
    const std::string& kind)
{
    std::lock_guard lock(mutex_);
    Result<WorkspaceId> result;
    if (!open_ || name.empty())
    {
        result.error = WorkspaceError::InvalidArgument;
        return result;
    }
    for (const auto& w : workspaces_)
    {
        if (w.name == name && w.status == WorkspaceStatus::Active)
        {
            result.error = WorkspaceError::AlreadyExists;
            return result;
        }
    }
    Workspace w;
    w.id = NewId();
    w.name = name;
    w.kind = kind;
    w.status = WorkspaceStatus::Active;
    w.created_unix_seconds = Now();
    workspaces_.push_back(w);
    if (Save() != WorkspaceError::Ok)
    {
        workspaces_.pop_back();
        result.error = WorkspaceError::StorageFailure;
        return result;
    }
    result.value = w.id;
    return result;
}

Result<Workspace> WorkspaceService::Get(const WorkspaceId& id) const
{
    std::lock_guard lock(mutex_);
    Result<Workspace> result;
    for (const auto& w : workspaces_)
    {
        if (w.id == id)
        {
            result.value = w;
            return result;
        }
    }
    result.error = WorkspaceError::NotFound;
    return result;
}

Result<std::vector<Workspace>> WorkspaceService::List() const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<Workspace>> result;
    result.value = workspaces_;
    return result;
}

Result<void> WorkspaceService::Archive(const WorkspaceId& id)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& w : workspaces_)
    {
        if (w.id == id)
        {
            w.status = WorkspaceStatus::Archived;
            result.error = Save();
            return result;
        }
    }
    result.error = WorkspaceError::NotFound;
    return result;
}


const char* WorkspaceService::StatusName(WorkspaceStatus s)
{
    switch (s)
    {
    case WorkspaceStatus::Active: return "active";
    case WorkspaceStatus::Archived: return "archived";
    case WorkspaceStatus::Suspended: return "suspended";
    }
    return "unknown";
}

Result<void> WorkspaceService::Activate(const WorkspaceId& id)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& w : workspaces_)
    {
        if (w.id == id)
        {
            if (w.status == WorkspaceStatus::Archived)
            {
                result.error = WorkspaceError::InvalidArgument;
                return result;
            }
            w.status = WorkspaceStatus::Active;
            if (Save() != WorkspaceError::Ok)
            {
                result.error = WorkspaceError::StorageFailure;
                return result;
            }
            return result;
        }
    }
    result.error = WorkspaceError::NotFound;
    return result;
}

Result<void> WorkspaceService::Suspend(const WorkspaceId& id)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& w : workspaces_)
    {
        if (w.id == id)
        {
            if (w.status == WorkspaceStatus::Archived)
            {
                result.error = WorkspaceError::InvalidArgument;
                return result;
            }
            w.status = WorkspaceStatus::Suspended;
            if (Save() != WorkspaceError::Ok)
            {
                result.error = WorkspaceError::StorageFailure;
                return result;
            }
            return result;
        }
    }
    result.error = WorkspaceError::NotFound;
    return result;
}

Result<Workspace> WorkspaceService::FindByName(const std::string& name) const
{
    std::lock_guard lock(mutex_);
    Result<Workspace> result;
    for (const auto& w : workspaces_)
    {
        if (w.name == name)
        {
            result.value = w;
            return result;
        }
    }
    result.error = WorkspaceError::NotFound;
    return result;
}

Result<std::vector<Workspace>> WorkspaceService::ListByStatus(WorkspaceStatus status) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<Workspace>> result;
    for (const auto& w : workspaces_)
    {
        if (w.status == status)
            result.value.push_back(w);
    }
    return result;
}

} // namespace sovereign::workspace
