#include "memory_engine.hpp"

#include <chrono>
#include <fstream>
#include <random>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace sovereign::memory
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
        if (c == '|' || c == '\\' || c == '\n' || c == ',')
        {
            o.push_back('\\');
            if (c == '\n') o.push_back('n');
            else o.push_back(c);
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

std::string JoinTags(const std::vector<std::string>& tags)
{
    std::string o;
    for (std::size_t i = 0; i < tags.size(); ++i)
    {
        if (i) o.push_back(',');
        o += Esc(tags[i]);
    }
    return o;
}

std::vector<std::string> SplitTags(const std::string& s)
{
    std::vector<std::string> tags;
    std::string cur;
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '\\' && i + 1 < s.size())
        {
            cur.push_back(s[i++]);
            cur.push_back(s[i]);
        }
        else if (s[i] == ',')
        {
            tags.push_back(Unesc(cur));
            cur.clear();
        }
        else cur.push_back(s[i]);
    }
    if (!cur.empty() || !s.empty()) tags.push_back(Unesc(cur));
    return tags;
}

} // namespace

RecordId MemoryEngine::NewId()
{
    RecordId id{};
    std::random_device rd;
    for (auto& x : id.bytes) x = static_cast<std::uint8_t>(rd());
    return id;
}

std::uint64_t MemoryEngine::Now()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

MemoryError MemoryEngine::Open(const std::filesystem::path& directory)
{
    std::lock_guard lock(mutex_);
    directory_ = directory;
    std::error_code ec;
    std::filesystem::create_directories(directory_, ec);
    if (ec) return MemoryError::StorageFailure;
    open_ = true;
    return Load();
}

void MemoryEngine::Close() noexcept
{
    std::lock_guard lock(mutex_);
    if (open_) static_cast<void>(Save());
    open_ = false;
}

MemoryError MemoryEngine::Load()
{
    records_.clear();
    std::ifstream in(directory_ / "records.dat");
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
        MemoryRecord r;
        if (!Unhex(p[0], r.id.bytes)) continue;
        static_cast<void>(Unhex(p[1], r.author.bytes));
        r.created_unix_seconds = std::stoull(p[2]);
        r.tags = SplitTags(p[3]);
        r.body = Unesc(p[4]);
        if (p.size() >= 6) r.importance = std::stod(p[5]);
        if (p.size() >= 7) r.last_access_unix_seconds = std::stoull(p[6]);
        if (p.size() >= 8) r.access_count = static_cast<std::uint32_t>(std::stoul(p[7]));
        records_.push_back(r);
    }
    return MemoryError::Ok;
}

MemoryError MemoryEngine::Save() const
{
    std::ofstream out(directory_ / "records.dat", std::ios::trunc);
    if (!out) return MemoryError::StorageFailure;
    for (const auto& r : records_)
    {
        out << Hex(r.id.bytes) << '|'
            << Hex(r.author.bytes) << '|'
            << r.created_unix_seconds << '|'
            << JoinTags(r.tags) << '|'
            << Esc(r.body) << '|'
            << r.importance << '|'
            << r.last_access_unix_seconds << '|'
            << r.access_count << '\n';
    }
    return MemoryError::Ok;
}

Result<RecordId> MemoryEngine::Append(const MemoryRecord& record)
{
    std::lock_guard lock(mutex_);
    Result<RecordId> result;
    if (!open_ || record.body.empty())
    {
        result.error = MemoryError::InvalidArgument;
        return result;
    }
    MemoryRecord r = record;
    bool nil_id = true;
    for (auto b : r.id.bytes) if (b) { nil_id = false; break; }
    if (nil_id)
    {
        r.id = NewId();
    }
    if (r.created_unix_seconds == 0)
    {
        r.created_unix_seconds = Now();
    }
    if (r.last_access_unix_seconds == 0)
        r.last_access_unix_seconds = r.created_unix_seconds;
    if (r.importance <= 0.0)
        r.importance = 0.5;
    records_.push_back(r);
    if (Save() != MemoryError::Ok)
    {
        records_.pop_back();
        result.error = MemoryError::StorageFailure;
        return result;
    }
    result.value = r.id;
    return result;
}

Result<MemoryRecord> MemoryEngine::Get(const RecordId& id) const
{
    std::lock_guard lock(mutex_);
    Result<MemoryRecord> result;
    for (const auto& r : records_)
    {
        if (r.id == id)
        {
            result.value = r;
            return result;
        }
    }
    result.error = MemoryError::NotFound;
    return result;
}

Result<std::vector<MemoryRecord>> MemoryEngine::ListByTag(
    const std::string& tag) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<MemoryRecord>> result;
    for (const auto& r : records_)
    {
        for (const auto& t : r.tags)
        {
            if (t == tag)
            {
                result.value.push_back(r);
                break;
            }
        }
    }
    return result;
}

Result<std::vector<MemoryRecord>> MemoryEngine::ListAll() const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<MemoryRecord>> result;
    result.value = records_;
    return result;
}


Result<std::vector<MemoryRecord>> MemoryEngine::Search(const std::string& query) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<MemoryRecord>> result;
    if (query.empty())
    {
        result.error = MemoryError::InvalidArgument;
        return result;
    }
    for (const auto& r : records_)
    {
        bool hit = r.body.find(query) != std::string::npos;
        if (!hit)
        {
            for (const auto& tag : r.tags)
            {
                if (tag.find(query) != std::string::npos)
                {
                    hit = true;
                    break;
                }
            }
        }
        if (hit)
            result.value.push_back(r);
    }
    return result;
}

double MemoryEngine::AgeScore(const MemoryRecord& r, std::uint64_t now_unix)
{
    const std::uint64_t base = r.last_access_unix_seconds
                                   ? r.last_access_unix_seconds
                                   : r.created_unix_seconds;
    const double age_days =
        now_unix > base ? static_cast<double>(now_unix - base) / 86400.0 : 0.0;
    // exponential decay half-life ~30 days, boosted by access_count
    const double decay = std::exp(-age_days / 30.0);
    const double boost = 1.0 + 0.05 * static_cast<double>(r.access_count);
    double s = r.importance * decay * boost;
    if (s < 0) s = 0;
    if (s > 1) s = 1;
    return s;
}

Result<void> MemoryEngine::Touch(const RecordId& id)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& r : records_)
    {
        if (r.id == id)
        {
            r.last_access_unix_seconds = Now();
            ++r.access_count;
            if (Save() != MemoryError::Ok)
                result.error = MemoryError::StorageFailure;
            return result;
        }
    }
    result.error = MemoryError::NotFound;
    return result;
}

Result<void> MemoryEngine::SetImportance(const RecordId& id, double importance)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    if (importance < 0.0 || importance > 1.0)
    {
        result.error = MemoryError::InvalidArgument;
        return result;
    }
    for (auto& r : records_)
    {
        if (r.id == id)
        {
            r.importance = importance;
            if (Save() != MemoryError::Ok)
                result.error = MemoryError::StorageFailure;
            return result;
        }
    }
    result.error = MemoryError::NotFound;
    return result;
}

Result<std::vector<ScoredRecord>> MemoryEngine::Ranked(std::size_t limit) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<ScoredRecord>> result;
    const auto now = Now();
    for (const auto& r : records_)
    {
        ScoredRecord s;
        s.record = r;
        s.score = AgeScore(r, now);
        result.value.push_back(std::move(s));
    }
    std::sort(result.value.begin(), result.value.end(),
              [](const ScoredRecord& a, const ScoredRecord& b) {
                  return a.score > b.score;
              });
    if (result.value.size() > limit)
        result.value.resize(limit);
    return result;
}

Result<std::uint32_t> MemoryEngine::PruneBelow(double min_score)
{
    std::lock_guard lock(mutex_);
    Result<std::uint32_t> result;
    const auto now = Now();
    std::vector<MemoryRecord> keep;
    std::uint32_t removed = 0;
    for (const auto& r : records_)
    {
        if (AgeScore(r, now) < min_score)
            ++removed;
        else
            keep.push_back(r);
    }
    records_.swap(keep);
    if (Save() != MemoryError::Ok)
    {
        result.error = MemoryError::StorageFailure;
        return result;
    }
    result.value = removed;
    return result;
}

} // namespace sovereign::memory
