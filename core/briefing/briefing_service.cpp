#include "briefing_service.hpp"

#include <sstream>

namespace sovereign::briefing
{

BriefingService::BriefingService(
    goals::GoalsService& goals,
    governance::GovernanceService& governance,
    memory::MemoryEngine& memory,
    workspace::WorkspaceService& workspace,
    audit::AuditLog& audit)
    : goals_(goals)
    , governance_(governance)
    , memory_(memory)
    , workspace_(workspace)
    , audit_(audit)
{
}

SystemBrief BriefingService::Build() const
{
    SystemBrief brief;

    BriefSection goals_sec;
    goals_sec.title = "Active goals";
    auto goals = goals_.ListActive();
    if (goals.Ok())
    {
        // Sort-ish by priority desc via simple insertion
        auto list = goals.value;
        for (std::size_t i = 0; i < list.size(); ++i)
        {
            for (std::size_t j = i + 1; j < list.size(); ++j)
            {
                if (list[j].priority > list[i].priority)
                    std::swap(list[i], list[j]);
            }
        }
        for (const auto& g : list)
        {
            goals_sec.lines.push_back(
                "[" + std::to_string(g.priority) + "] " + g.title +
                (g.workspace_name.empty() ? "" : (" @" + g.workspace_name)));
        }
    }
    if (goals_sec.lines.empty())
        goals_sec.lines.push_back("(none)");
    brief.sections.push_back(goals_sec);

    BriefSection gov;
    gov.title = "Pending governance decisions";
    auto pending = governance_.ListPending();
    if (pending.Ok())
    {
        for (const auto& d : pending.value)
            gov.lines.push_back(d.action + " — " + d.summary);
    }
    if (gov.lines.empty())
        gov.lines.push_back("(none)");
    brief.sections.push_back(gov);

    BriefSection ws;
    ws.title = "Workspaces";
    auto wlist = workspace_.List();
    if (wlist.Ok())
    {
        for (const auto& w : wlist.value)
        {
            if (w.status == workspace::WorkspaceStatus::Active)
                ws.lines.push_back(w.name + " (" + w.kind + ")");
        }
    }
    if (ws.lines.empty())
        ws.lines.push_back("(none)");
    brief.sections.push_back(ws);

    BriefSection mem;
    mem.title = "Recent memory";
    auto all = memory_.ListAll();
    if (all.Ok())
    {
        const auto& v = all.value;
        const std::size_t start = v.size() > 5 ? v.size() - 5 : 0;
        for (std::size_t i = start; i < v.size(); ++i)
        {
            std::string tags;
            for (std::size_t t = 0; t < v[i].tags.size(); ++t)
            {
                if (t)
                    tags += ",";
                tags += v[i].tags[t];
            }
            mem.lines.push_back(
                (tags.empty() ? "" : ("#" + tags + " ")) + v[i].body);
        }
    }
    if (mem.lines.empty())
        mem.lines.push_back("(none)");
    brief.sections.push_back(mem);

    BriefSection aud;
    aud.title = "Recent audit";
    auto al = audit_.List(5);
    if (al.Ok())
    {
        for (const auto& e : al.value)
            aud.lines.push_back(e.category + "/" + e.action + " " + e.detail);
    }
    if (aud.lines.empty())
        aud.lines.push_back("(none)");
    brief.sections.push_back(aud);

    std::ostringstream oss;
    oss << "=== Sovereign system brief ===\n";
    for (const auto& sec : brief.sections)
    {
        oss << "\n## " << sec.title << "\n";
        for (const auto& line : sec.lines)
            oss << "  - " << line << "\n";
    }
    brief.rendered_text = oss.str();
    return brief;
}

} // namespace sovereign::briefing
