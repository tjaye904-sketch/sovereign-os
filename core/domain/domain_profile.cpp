#include "domain_profile.hpp"

namespace sovereign::domain
{

std::vector<DomainProfile> BuiltinProfiles()
{
    return {
        {"development",
         "You are the Sovereign Development workspace agent. Focus on code, "
         "builds, tests, and technical design. Never claim to have modified "
         "systems without Governance approval.",
         {"record_memory", "code_search", "run_tests"},
         {"transfer", "deploy_production"}},
        {"knowledge",
         "You are the Sovereign Knowledge workspace agent. Organize research, "
         "notes, and citations. Prefer durable Memory and Archive references.",
         {"record_memory", "semantic_query", "archive_store"},
         {"deploy", "transfer"}},
        {"business",
         "You are the Sovereign Business workspace agent. Track vendors, "
         "commitments, and operational status. Flag items needing human "
         "approval.",
         {"record_memory", "semantic_query"},
         {"transfer", "deploy"}},
        {"finance",
         "You are the Sovereign Finance workspace agent. Discuss projections "
         "and reviews only. Never execute money movement; require Governance.",
         {"record_memory", "semantic_query"},
         {"transfer", "deploy", "delete"}},
        {"health",
         "You are the Sovereign Health workspace agent. Support reflection and "
         "tracking. Not medical advice. No automated external actions.",
         {"record_memory"},
         {"deploy", "transfer", "delete"}},
        {"infrastructure",
         "You are the Sovereign Infrastructure workspace agent. Reason about "
         "servers and reliability. Production changes require Governance.",
         {"record_memory", "run_tests"},
         {"transfer"}},
        {"life",
         "You are the Sovereign Life workspace agent. Help with personal "
         "logistics and reminders. Consequential actions need human approval.",
         {"record_memory"},
         {"deploy", "transfer"}},
    };
}

const DomainProfile* FindProfile(const std::string& name)
{
    static const std::vector<DomainProfile> kProfiles = BuiltinProfiles();
    for (const auto& p : kProfiles)
    {
        if (p.name == name)
            return &p;
    }
    return nullptr;
}

bool ToolAllowed(const DomainProfile& profile, const std::string& tool_name)
{
    for (const auto& t : profile.allowed_tools)
    {
        if (t == tool_name)
            return true;
    }
    return false;
}

bool ActionDenied(const DomainProfile& profile, const std::string& action)
{
    for (const auto& a : profile.denied_actions)
    {
        if (a == action)
            return true;
    }
    return false;
}


// L0.2: blueprint workspace id list (status may be stub — see COMPONENT_REGISTRY.yaml)
static const char* kWorkspaceNames16[] = {
    "development",
    "knowledge",
    "business",
    "finance",
    "life",
    "health",
    "research",
    "security",
    "infrastructure",
    "automation",
    "robotics",
    "communications",
    "creative",
    "intelligence",
    "governance",
    "federation",
    nullptr
};

} // namespace sovereign::domain
