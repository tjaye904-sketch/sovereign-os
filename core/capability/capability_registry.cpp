#include "capability_registry.hpp"

#include <fstream>

namespace sovereign::capability
{
namespace
{

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

CapabilityError CapabilityRegistry::Open(const std::filesystem::path& data_root)
{
    std::lock_guard lock(mutex_);
    std::error_code ec;
    auto dir = data_root / "capability";
    std::filesystem::create_directories(dir, ec);
    if (ec)
        return CapabilityError::StorageFailure;
    path_ = dir / "registry.dat";
    open_ = true;
    return Load();
}

void CapabilityRegistry::Close() noexcept
{
    std::lock_guard lock(mutex_);
    if (open_)
        static_cast<void>(Save());
    open_ = false;
}

CapabilityError CapabilityRegistry::Load()
{
    items_.clear();
    std::ifstream in(path_);
    if (!in)
        return CapabilityError::Ok;
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
                p.push_back(Unesc(cur));
                cur.clear();
            }
            else
                cur.push_back(line[i]);
        }
        p.push_back(Unesc(cur));
        if (p.size() < 4)
            continue;
        Capability c;
        c.id = p[0];
        c.description = p[1];
        c.enabled = (p[2] == "1");
        c.source = p[3];
        items_.push_back(std::move(c));
    }
    return CapabilityError::Ok;
}

CapabilityError CapabilityRegistry::Save() const
{
    std::ofstream out(path_, std::ios::trunc);
    if (!out)
        return CapabilityError::StorageFailure;
    for (const auto& c : items_)
    {
        out << Esc(c.id) << '|' << Esc(c.description) << '|'
            << (c.enabled ? '1' : '0') << '|' << Esc(c.source) << '\n';
    }
    return CapabilityError::Ok;
}

Result<std::size_t> CapabilityRegistry::SeedDefaults()
{
    std::lock_guard lock(mutex_);
    Result<std::size_t> r;
    if (!open_)
    {
        r.error = CapabilityError::NotOpen;
        return r;
    }
    const struct
    {
        const char* id;
        const char* desc;
    } seeds[] = {
        {"host.cli", "Local sovereign-cli surface"},
        {"host.daemon", "User sovereign-daemon orchestrator"},
        {"host.computer_use", "Browser/desktop computer-use server"},
        {"host.sys", "Host OS actuation (app open, services)"},
        {"host.mission_control", "Mission Control hub UI"},
        {"model.chat", "Ollama chat routing"},
        {"model.embed", "Embedding model routing"},
        {"cognitive.plan", "Cognitive plan / loop"},
        {"cognitive.orchestrate", "Orchestration engine"},
        {"governance.approve", "Governance approve/reject path"},
        {"memory.facets", "Faceted memory"},
        {"secrets.store", "Secret manager"},
        {"workspace.controller", "Workspace activate/suspend"},
        {"sync.peer", "Peer bundle export/import"},
        {"voice.speak", "TTS output"},
        {"voice.listen", "STT input"},
    };
    std::size_t added = 0;
    for (const auto& s : seeds)
    {
        bool found = false;
        for (const auto& it : items_)
        {
            if (it.id == s.id)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            Capability c;
            c.id = s.id;
            c.description = s.desc;
            c.enabled = true;
            c.source = "seed";
            items_.push_back(std::move(c));
            ++added;
        }
    }
    if (Save() != CapabilityError::Ok)
    {
        r.error = CapabilityError::StorageFailure;
        return r;
    }
    r.value = added;
    return r;

        {"host.execute", "Governed execute"},
        {"host.purpose", "Purpose automation"},
        {"host.voice", "Voice"},
}

Result<void> CapabilityRegistry::Register(const Capability& cap)
{
    std::lock_guard lock(mutex_);
    if (!open_)
        return Result<void>::Err(CapabilityError::NotOpen);
    if (cap.id.empty())
        return Result<void>::Err(CapabilityError::InvalidArgument);
    for (auto& it : items_)
    {
        if (it.id == cap.id)
        {
            it.description = cap.description.empty() ? it.description : cap.description;
            it.enabled = cap.enabled;
            it.source = cap.source.empty() ? it.source : cap.source;
            if (Save() != CapabilityError::Ok)
                return Result<void>::Err(CapabilityError::StorageFailure);
            return Result<void>::OkRes();
        }
    }
    items_.push_back(cap);
    if (Save() != CapabilityError::Ok)
        return Result<void>::Err(CapabilityError::StorageFailure);
    return Result<void>::OkRes();
}

Result<void> CapabilityRegistry::SetEnabled(const std::string& id, bool enabled)
{
    std::lock_guard lock(mutex_);
    if (!open_)
        return Result<void>::Err(CapabilityError::NotOpen);
    for (auto& it : items_)
    {
        if (it.id == id)
        {
            it.enabled = enabled;
            if (Save() != CapabilityError::Ok)
                return Result<void>::Err(CapabilityError::StorageFailure);
            return Result<void>::OkRes();
        }
    }
    return Result<void>::Err(CapabilityError::NotFound);
}

Result<Capability> CapabilityRegistry::Get(const std::string& id) const
{
    std::lock_guard lock(mutex_);
    Result<Capability> r;
    if (!open_)
    {
        r.error = CapabilityError::NotOpen;
        return r;
    }
    for (const auto& it : items_)
    {
        if (it.id == id)
        {
            r.value = it;
            return r;
        }
    }
    r.error = CapabilityError::NotFound;
    return r;
}

Result<std::vector<Capability>> CapabilityRegistry::List() const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<Capability>> r;
    if (!open_)
    {
        r.error = CapabilityError::NotOpen;
        return r;
    }
    r.value = items_;
    return r;
}

Result<bool> CapabilityRegistry::IsEnabled(const std::string& id) const
{
    std::lock_guard lock(mutex_);
    Result<bool> r;
    if (!open_)
    {
        r.error = CapabilityError::NotOpen;
        return r;
    }
    for (const auto& it : items_)
    {
        if (it.id == id)
        {
            r.value = it.enabled;
            return r;
        }
    }
    // Unknown capability → fail closed
    r.value = false;
    return r;
}

} // namespace sovereign::capability
