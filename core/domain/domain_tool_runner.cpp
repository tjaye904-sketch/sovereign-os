#include "domain_tool_runner.hpp"

namespace sovereign::domain
{

DomainToolRunner::DomainToolRunner(
    memory::MemoryEngine& memory,
    semantic::SemanticStore& semantic)
    : memory_(memory)
    , semantic_(semantic)
{
}

ToolCallResult DomainToolRunner::Run(
    const DomainProfile& profile,
    const ToolCallRequest& call)
{
    ToolCallResult r;
    if (!ToolAllowed(profile, call.name))
    {
        r.allowed = false;
        r.ok = false;
        r.detail = "tool not allowed in domain " + profile.name;
        return r;
    }
    r.allowed = true;

    if (call.name == "record_memory")
    {
        memory::MemoryRecord rec;
        rec.tags = {"domain", profile.name, "tool"};
        rec.body =
            call.arguments_json.empty() ? "(empty)" : call.arguments_json;
        auto app = memory_.Append(rec);
        r.ok = app.Ok();
        r.detail = r.ok ? "memory_recorded" : "memory_failed";
        return r;
    }

    if (call.name == "semantic_query")
    {
        const std::string q =
            call.arguments_json.empty() ? profile.name : call.arguments_json;
        auto qres = semantic_.Query(q, 3);
        if (!qres.Ok())
        {
            r.ok = false;
            r.detail = "semantic_query_failed";
            return r;
        }
        r.ok = true;
        r.detail = "hits=" + std::to_string(qres.value.size());
        for (const auto& s : qres.value)
        {
            r.detail += "\n";
            r.detail += std::to_string(s.score) + " " + s.record.text;
        }
        return r;
    }

    if (call.name == "archive_store" || call.name == "code_search" ||
        call.name == "run_tests")
    {
        r.ok = true;
        r.detail = "acknowledged_stub:" + call.name;
        return r;
    }

    r.ok = false;
    r.detail = "unknown tool";
    return r;
}

} // namespace sovereign::domain
