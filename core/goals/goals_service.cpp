#include "goals_service.hpp"

#include <chrono>
#include <fstream>
#include <random>

namespace sovereign::goals
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

GoalId GoalsService::NewId()
{
    GoalId id{};
    std::random_device rd;
    for (auto& x : id.bytes)
        x = static_cast<std::uint8_t>(rd());
    return id;
}

std::uint64_t GoalsService::Now()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

GoalsError GoalsService::Open(const std::filesystem::path& directory)
{
    std::lock_guard lock(mutex_);
    directory_ = directory;
    std::error_code ec;
    std::filesystem::create_directories(directory_, ec);
    if (ec)
        return GoalsError::StorageFailure;
    open_ = true;
    return Load();
}

void GoalsService::Close() noexcept
{
    std::lock_guard lock(mutex_);
    if (open_)
        static_cast<void>(Save());
    open_ = false;
}

GoalsError GoalsService::Load()
{
    goals_.clear();
    std::ifstream in(directory_ / "goals.dat");
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
        if (p.size() < 10)
            continue;
        Goal g;
        if (!Unhex(p[0], g.id.bytes))
            continue;
        Unhex(p[1], g.owner.bytes);
        g.title = Unesc(p[2]);
        g.description = Unesc(p[3]);
        g.status = static_cast<GoalStatus>(std::stoi(p[4]));
        g.horizon = static_cast<GoalHorizon>(std::stoi(p[5]));
        g.workspace_name = Unesc(p[6]);
        g.priority = std::stoi(p[7]);
        g.created_unix_seconds = std::stoull(p[8]);
        g.updated_unix_seconds = std::stoull(p[9]);
        goals_.push_back(g);
    }
    return GoalsError::Ok;
}

GoalsError GoalsService::Save() const
{
    std::ofstream out(directory_ / "goals.dat", std::ios::trunc);
    if (!out)
        return GoalsError::StorageFailure;
    for (const auto& g : goals_)
    {
        out << Hex(g.id.bytes) << '|' << Hex(g.owner.bytes) << '|'
            << Esc(g.title) << '|' << Esc(g.description) << '|'
            << static_cast<int>(g.status) << '|' << static_cast<int>(g.horizon)
            << '|' << Esc(g.workspace_name) << '|' << g.priority << '|'
            << g.created_unix_seconds << '|' << g.updated_unix_seconds << '\n';
    }
    return GoalsError::Ok;
}

Result<GoalId> GoalsService::Create(const Goal& goal)
{
    std::lock_guard lock(mutex_);
    Result<GoalId> result;
    if (!open_ || goal.title.empty())
    {
        result.error = GoalsError::InvalidArgument;
        return result;
    }
    Goal g = goal;
    bool nil = true;
    for (auto b : g.id.bytes)
        if (b)
        {
            nil = false;
            break;
        }
    if (nil)
        g.id = NewId();
    const auto now = Now();
    if (!g.created_unix_seconds)
        g.created_unix_seconds = now;
    g.updated_unix_seconds = now;
    if (g.priority < 0)
        g.priority = 0;
    if (g.priority > 100)
        g.priority = 100;
    goals_.push_back(g);
    if (Save() != GoalsError::Ok)
    {
        goals_.pop_back();
        result.error = GoalsError::StorageFailure;
        return result;
    }
    result.value = g.id;
    return result;
}

Result<Goal> GoalsService::Get(const GoalId& id) const
{
    std::lock_guard lock(mutex_);
    Result<Goal> result;
    for (const auto& g : goals_)
    {
        if (g.id == id)
        {
            result.value = g;
            return result;
        }
    }
    result.error = GoalsError::NotFound;
    return result;
}

Result<std::vector<Goal>> GoalsService::ListActive() const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<Goal>> result;
    for (const auto& g : goals_)
    {
        if (g.status == GoalStatus::Active)
            result.value.push_back(g);
    }
    return result;
}

Result<std::vector<Goal>> GoalsService::ListAll() const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<Goal>> result;
    result.value = goals_;
    return result;
}

Result<void> GoalsService::SetStatus(const GoalId& id, GoalStatus status)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& g : goals_)
    {
        if (g.id == id)
        {
            g.status = status;
            g.updated_unix_seconds = Now();
            result.error = Save();
            return result;
        }
    }
    result.error = GoalsError::NotFound;
    return result;
}

Result<void> GoalsService::SetPriority(const GoalId& id, int priority)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    if (priority < 0)
        priority = 0;
    if (priority > 100)
        priority = 100;
    for (auto& g : goals_)
    {
        if (g.id == id)
        {
            g.priority = priority;
            g.updated_unix_seconds = Now();
            result.error = Save();
            return result;
        }
    }
    result.error = GoalsError::NotFound;
    return result;
}

} // namespace sovereign::goals
