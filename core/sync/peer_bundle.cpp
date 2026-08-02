#include "peer_bundle.hpp"

#include <chrono>
#include <fstream>
#include <sstream>

namespace sovereign::sync
{
namespace
{

std::uint64_t Now()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

std::string Hex16(const std::array<std::uint8_t, 16>& b)
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

std::string Esc(const std::string& s)
{
    std::string o;
    for (char c : s)
    {
        if (c == '\n')
            o += "\\n";
        else if (c == '|')
            o += "\\|";
        else if (c == '\\')
            o += "\\\\";
        else
            o.push_back(c);
    }
    return o;
}

} // namespace

PeerBundleService::PeerBundleService(
    goals::GoalsService& goals,
    memory::MemoryEngine& memory,
    semantic::SemanticStore& semantic,
    audit::AuditLog& audit,
    SyncService& journal)
    : goals_(goals)
    , memory_(memory)
    , semantic_(semantic)
    , audit_(audit)
    , journal_(journal)
{
}

PeerResult<PeerBundleSummary> PeerBundleService::ExportTo(
    const std::filesystem::path& dest_dir,
    const PrincipalId& actor)
{
    PeerResult<PeerBundleSummary> result;
    if (dest_dir.empty())
    {
        result.error = PeerError::InvalidArgument;
        return result;
    }
    std::error_code ec;
    std::filesystem::create_directories(dest_dir, ec);
    if (ec)
    {
        result.error = PeerError::IOError;
        return result;
    }
    const auto bundle =
        dest_dir / ("sovereign-bundle-" + std::to_string(Now()));
    std::filesystem::create_directories(bundle, ec);
    if (ec)
    {
        result.error = PeerError::IOError;
        return result;
    }

    PeerBundleSummary sum;
    sum.path = bundle.string();

    // MANIFEST
    {
        std::ofstream m(bundle / "MANIFEST.txt");
        m << "sovereign-peer-bundle-v1\n";
        m << "created_unix=" << Now() << "\n";
        m << "note=no_credentials_no_auto_execute\n";
    }

    // goals
    {
        std::ofstream out(bundle / "goals.tsv");
        auto g = goals_.ListAll();
        if (g.Ok())
        {
            for (const auto& goal : g.value)
            {
                out << Hex16(goal.id.bytes) << '\t' << goal.priority << '\t'
                    << static_cast<int>(goal.status) << '\t'
                    << Esc(goal.title) << '\t' << Esc(goal.description)
                    << '\n';
                ++sum.goals;
            }
        }
    }

    // memory
    {
        std::ofstream out(bundle / "memory.tsv");
        auto m = memory_.ListAll();
        if (m.Ok())
        {
            for (const auto& rec : m.value)
            {
                std::string tags;
                for (std::size_t i = 0; i < rec.tags.size(); ++i)
                {
                    if (i)
                        tags += ",";
                    tags += rec.tags[i];
                }
                out << Esc(tags) << '\t' << Esc(rec.body) << '\n';
                ++sum.memory;
            }
        }
    }

    // semantic (text + tags only; re-embed on import)
    {
        std::ofstream out(bundle / "semantic.tsv");
        // SemanticStore has no ListAll public — query via size/unavailable;
        // use memory-like: add ListTexts later. For v1 export from memory
        // already covers narrative; semantic re-index on import of memory.
        static_cast<void>(out);
    }

    // audit tail
    {
        std::ofstream out(bundle / "audit.tsv");
        auto a = audit_.List(100);
        if (a.Ok())
        {
            for (const auto& e : a.value)
            {
                out << e.unix_seconds << '\t' << Esc(e.category) << '\t'
                    << Esc(e.action) << '\t' << Esc(e.detail) << '\n';
                ++sum.audit;
            }
        }
    }

    static_cast<void>(journal_.RecordExport(
        actor, "peer-folder", sum.path));
    result.value = sum;
    return result;
}

PeerResult<PeerBundleSummary> PeerBundleService::ImportFrom(
    const std::filesystem::path& bundle_dir,
    const PrincipalId& actor)
{
    PeerResult<PeerBundleSummary> result;
    if (bundle_dir.empty() ||
        !std::filesystem::exists(bundle_dir / "MANIFEST.txt"))
    {
        result.error = PeerError::SchemaError;
        return result;
    }

    PeerBundleSummary sum;
    sum.path = bundle_dir.string();

    // goals
    {
        std::ifstream in(bundle_dir / "goals.tsv");
        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty())
                continue;
            // id\tpriority\tstatus\ttitle\tdesc
            std::vector<std::string> p;
            std::string cur;
            for (char c : line)
            {
                if (c == '\t')
                {
                    p.push_back(cur);
                    cur.clear();
                }
                else
                    cur.push_back(c);
            }
            p.push_back(cur);
            if (p.size() < 4)
                continue;
            goals::Goal g;
            g.title = p[3];
            if (p.size() >= 5)
                g.description = p[4];
            g.priority = std::stoi(p[1]);
            g.status = static_cast<goals::GoalStatus>(std::stoi(p[2]));
            g.owner = actor;
            auto cr = goals_.Create(g);
            if (cr.Ok())
                ++sum.goals;
        }
    }

    // memory append
    {
        std::ifstream in(bundle_dir / "memory.tsv");
        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty())
                continue;
            auto tab = line.find('\t');
            if (tab == std::string::npos)
                continue;
            memory::MemoryRecord rec;
            std::string tags = line.substr(0, tab);
            rec.body = line.substr(tab + 1);
            std::string t;
            for (char c : tags)
            {
                if (c == ',')
                {
                    if (!t.empty())
                        rec.tags.push_back(t);
                    t.clear();
                }
                else
                    t.push_back(c);
            }
            if (!t.empty())
                rec.tags.push_back(t);
            rec.tags.push_back("peer-import");
            auto app = memory_.Append(rec);
            if (app.Ok())
            {
                ++sum.memory;
                static_cast<void>(
                    semantic_.Upsert(rec.body, rec.tags));
                ++sum.semantic;
            }
        }
    }

    // audit note of import (do not replay foreign audit as local truth)
    {
        audit::AuditEntry e;
        e.category = "sync";
        e.action = "peer_import";
        e.actor = actor;
        e.subject = bundle_dir.string();
        e.detail = "imported goals=" + std::to_string(sum.goals) +
                   " memory=" + std::to_string(sum.memory);
        static_cast<void>(audit_.Append(e));
        ++sum.audit;
    }

    static_cast<void>(
        journal_.RecordImport(actor, "peer-folder", sum.path));
    result.value = sum;
    return result;
}

} // namespace sovereign::sync
