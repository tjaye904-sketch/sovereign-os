#include "semantic_store.hpp"
#include "local_embedder.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <random>
#include <sstream>

namespace sovereign::semantic
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
std::string EmbToStr(const Embedding& e)
{
    std::ostringstream oss;
    for (std::size_t i = 0; i < e.v.size(); ++i)
    {
        if (i)
            oss << ',';
        oss << e.v[i];
    }
    return oss.str();
}
bool StrToEmb(const std::string& s, Embedding& e)
{
    e.v.clear();
    if (s.empty())
        return false;
    std::stringstream ss(s);
    std::string part;
    while (std::getline(ss, part, ','))
        e.v.push_back(std::strtof(part.c_str(), nullptr));
    return !e.v.empty();
}
} // namespace

SemanticRecordId SemanticStore::NewId()
{
    SemanticRecordId id{};
    std::random_device rd;
    for (auto& x : id.bytes)
        x = static_cast<std::uint8_t>(rd());
    return id;
}

std::uint64_t SemanticStore::Now()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

SemanticError SemanticStore::Open(const std::filesystem::path& directory)
{
    std::lock_guard lock(mutex_);
    directory_ = directory;
    std::error_code ec;
    std::filesystem::create_directories(directory_, ec);
    if (ec)
        return SemanticError::StorageFailure;
    open_ = true;
    return Load();
}

void SemanticStore::Close() noexcept
{
    std::lock_guard lock(mutex_);
    if (open_)
        static_cast<void>(Save());
    open_ = false;
}

SemanticError SemanticStore::Load()
{
    records_.clear();
    std::ifstream in(directory_ / "semantic.dat");
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
        // id|author|text|embedding|tags|created|[model]
        if (p.size() < 6)
            continue;
        SemanticRecord r;
        if (!Unhex(p[0], r.id.bytes))
            continue;
        Unhex(p[1], r.author.bytes);
        r.text = Unesc(p[2]);
        if (!StrToEmb(p[3], r.embedding))
        {
            std::string used;
            r.embedding = EmbedText(r.text, &used);
            r.embed_model = used;
        }
        {
            std::string tags = Unesc(p[4]);
            std::string t;
            for (char c : tags)
            {
                if (c == ',')
                {
                    if (!t.empty())
                        r.tags.push_back(t);
                    t.clear();
                }
                else
                    t.push_back(c);
            }
            if (!t.empty())
                r.tags.push_back(t);
        }
        r.created_unix_seconds = std::stoull(p[5]);
        if (p.size() >= 7)
            r.embed_model = Unesc(p[6]);
        records_.push_back(r);
    }
    return SemanticError::Ok;
}

SemanticError SemanticStore::Save() const
{
    std::ofstream out(directory_ / "semantic.dat", std::ios::trunc);
    if (!out)
        return SemanticError::StorageFailure;
    for (const auto& r : records_)
    {
        std::string tags;
        for (std::size_t i = 0; i < r.tags.size(); ++i)
        {
            if (i)
                tags.push_back(',');
            tags += r.tags[i];
        }
        out << Hex(r.id.bytes) << '|' << Hex(r.author.bytes) << '|'
            << Esc(r.text) << '|' << EmbToStr(r.embedding) << '|' << Esc(tags)
            << '|' << r.created_unix_seconds << '|' << Esc(r.embed_model)
            << '\n';
    }
    return SemanticError::Ok;
}

Result<SemanticRecordId> SemanticStore::Upsert(
    const std::string& text,
    const std::vector<std::string>& tags,
    const PrincipalId& author)
{
    std::lock_guard lock(mutex_);
    Result<SemanticRecordId> result;
    if (!open_ || text.empty())
    {
        result.error = SemanticError::InvalidArgument;
        return result;
    }
    SemanticRecord r;
    r.id = NewId();
    r.author = author;
    r.text = text;
    r.tags = tags;
    std::string used;
    r.embedding = EmbedText(text, &used);
    r.embed_model = used;
    r.created_unix_seconds = Now();
    records_.push_back(r);
    if (Save() != SemanticError::Ok)
    {
        records_.pop_back();
        result.error = SemanticError::StorageFailure;
        return result;
    }
    result.value = r.id;
    return result;
}

Result<std::vector<ScoredRecord>> SemanticStore::Query(
    const std::string& query,
    std::size_t top_k) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<ScoredRecord>> result;
    if (query.empty())
    {
        result.error = SemanticError::InvalidArgument;
        return result;
    }
    std::string used;
    const Embedding q = EmbedText(query, &used);
    std::vector<ScoredRecord> scored;
    scored.reserve(records_.size());
    for (const auto& r : records_)
    {
        // Prefer same-model comparisons; still score cross-model partially
        ScoredRecord s;
        s.record = r;
        s.score = CosineSimilarity(q, r.embedding);
        scored.push_back(s);
    }
    std::sort(scored.begin(), scored.end(),
              [](const ScoredRecord& a, const ScoredRecord& b) {
                  return a.score > b.score;
              });
    if (scored.size() > top_k)
        scored.resize(top_k);
    result.value = std::move(scored);
    return result;
}

Result<std::size_t> SemanticStore::Size() const
{
    std::lock_guard lock(mutex_);
    Result<std::size_t> result;
    result.value = records_.size();
    return result;
}

} // namespace sovereign::semantic
