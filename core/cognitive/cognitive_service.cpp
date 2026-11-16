#include "cognitive_service.hpp"

#include <algorithm>
#include <chrono>
#include <random>

namespace sovereign::cognitive
{

AnalysisId CognitiveService::NewId()
{
    AnalysisId id{};
    std::random_device rd;
    for (auto& b : id.bytes)
    {
        b = static_cast<std::uint8_t>(rd());
    }
    return id;
}

std::uint64_t CognitiveService::Now()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

CognitiveService::CognitiveService(
    sovereign::governance::GovernanceService& governance,
    sovereign::memory::MemoryEngine* memory,
    sovereign::model_router::ModelRouter* models,
    std::string default_model)
    : governance_(governance)
    , memory_(memory)
    , curiosity_engine_(memory)
    , reasoning_engine_(memory)
    , decision_engine_(&governance)
    , reflection_engine_(memory)
    , planning_engine_(&governance, memory, default_model)
    , orchestration_engine_(&governance, memory)
    , models_(models)
    , default_model_(std::move(default_model))
{
}

Result<CuriosityProposal> CognitiveService::ProposeCuriosity(
    const PrincipalId& requester,
    const std::string& topic,
    const std::vector<std::string>& questions)
{
    return curiosity_engine_.Propose(requester, topic, questions);
}

Result<ReasoningResult> CognitiveService::Reason(
    const PrincipalId& requester,
    const std::string& question,
    const std::vector<std::string>& points_for,
    const std::vector<std::string>& points_against,
    const std::string& summary)
{
    return reasoning_engine_.Reason(requester, question, points_for, points_against, summary);
}

Result<DecisionRecommendation> CognitiveService::RecommendAction(
    const PrincipalId& requester,
    const std::string& action,
    const std::string& summary,
    const std::string& evidence_ref)
{
    return decision_engine_.RecommendAction(requester, action, summary, evidence_ref);
}

Result<void> CognitiveService::AssertApproved(
    const std::array<std::uint8_t, 16>& governance_decision_id) const
{
    return decision_engine_.AssertApproved(governance_decision_id);
}

Result<ReflectionNote> CognitiveService::Reflect(
    const PrincipalId& author,
    const std::string& subject,
    const std::string& body)
{
    return reflection_engine_.Reflect(author, subject, body);
}

Result<ExecutionPlan> CognitiveService::BuildPlan(
    const PrincipalId& requester,
    const std::string& goal)
{
    auto result = planning_engine_.BuildPlan(requester, goal);
    if (result.Ok())
        EnrichPlanWithModel(result.value);
    return result;
}

void CognitiveService::EnrichPlanWithModel(ExecutionPlan& plan) const
{
    if (!models_)
    {
        plan.enrichment_status = "no_router";
        return;
    }

    auto health = models_->Health();
    if (!health.Ok() || !health.value)
    {
        plan.enrichment_status = "model_unavailable";
        plan.uncertainty_notes += " [ollama unreachable]";
        return;
    }

    // Prefer configured model; fall back to first listed tag if chat fails
    std::vector<std::string> candidates;
    if (!default_model_.empty())
        candidates.push_back(default_model_);
    auto listed = models_->ListModels();
    if (listed.Ok())
    {
        for (const auto& m : listed.value)
        {
            if (m.name.empty())
                continue;
            bool dup = false;
            for (const auto& c : candidates)
                if (c == m.name)
                {
                    dup = true;
                    break;
                }
            if (!dup)
                candidates.push_back(m.name);
            if (candidates.size() >= 2)
                break;
        }
    }
    if (candidates.empty())
        candidates.push_back("openclaw");

    sovereign::model_router::ChatRequest req;
    req.temperature = 0.2;
    req.num_predict = 220;
    sovereign::model_router::ChatMessage sys;
    sys.role = "system";
    sys.content =
        "You refine an execution plan for Sovereign OS. "
        "Output ONLY lines in this exact format:\n"
        "STEP: action | detail | gov=0\n"
        "STEP: action | detail | gov=1\n"
        "CONF: 0.75\n"
        "NOTE: short uncertainty text\n"
        "No markdown fences. Max 8 STEP lines.";
    sovereign::model_router::ChatMessage user;
    user.role = "user";
    user.content = "Goal: " + plan.goal + "\nCurrent steps:\n";
    for (const auto& s : plan.steps)
        user.content += "- " + s.action + ": " + s.detail + "\n";
    req.messages = {sys, user};

    sovereign::model_router::ChatResponse response;
    bool got = false;
    for (const auto& model : candidates)
    {
        req.model = model;
        auto resp = models_->Chat(req);
        if (resp.Ok() && resp.value.ok && !resp.value.content.empty())
        {
            response = std::move(resp.value);
            got = true;
            break;
        }
    }
    if (!got)
    {
        plan.enrichment_status = "model_unavailable";
        plan.uncertainty_notes += " [model chat failed]";
        return;
    }

    const std::string& text = response.content;
    std::vector<PlanStep> parsed;
    for (std::size_t i = 0; i < text.size();)
    {
        auto nl = text.find('\n', i);
        std::string line =
            text.substr(i, nl == std::string::npos ? std::string::npos : nl - i);
        i = nl == std::string::npos ? text.size() : nl + 1;
        while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
            line.pop_back();
        // strip common markdown prefixes
        while (!line.empty() && (line[0] == '`' || line[0] == '*' || line[0] == ' '))
            line.erase(line.begin());
        if (line.rfind("STEP:", 0) == 0 || line.rfind("step:", 0) == 0)
        {
            PlanStep st;
            auto body = line.substr(5);
            while (!body.empty() && body[0] == ' ')
                body.erase(body.begin());
            auto p1 = body.find('|');
            if (p1 == std::string::npos)
            {
                st.action = "step";
                st.detail = body;
            }
            else
            {
                st.action = body.substr(0, p1);
                while (!st.action.empty() && st.action.back() == ' ')
                    st.action.pop_back();
                auto rest = body.substr(p1 + 1);
                while (!rest.empty() && rest[0] == ' ')
                    rest.erase(rest.begin());
                auto p2 = rest.find('|');
                if (p2 == std::string::npos)
                    st.detail = rest;
                else
                {
                    st.detail = rest.substr(0, p2);
                    while (!st.detail.empty() && st.detail.back() == ' ')
                        st.detail.pop_back();
                    auto g = rest.substr(p2 + 1);
                    st.requires_governance =
                        g.find('1') != std::string::npos ||
                        g.find("gov=1") != std::string::npos;
                }
            }
            if (!st.action.empty())
                parsed.push_back(std::move(st));
        }
        else if (line.rfind("CONF:", 0) == 0 || line.rfind("conf:", 0) == 0)
        {
            try
            {
                plan.confidence = std::stod(line.substr(5));
                if (plan.confidence < 0)
                    plan.confidence = 0;
                if (plan.confidence > 1)
                    plan.confidence = 1;
            }
            catch (...)
            {
            }
        }
        else if (line.rfind("NOTE:", 0) == 0 || line.rfind("note:", 0) == 0)
        {
            plan.uncertainty_notes = line.substr(5);
            while (!plan.uncertainty_notes.empty() &&
                   plan.uncertainty_notes[0] == ' ')
                plan.uncertainty_notes.erase(plan.uncertainty_notes.begin());
        }
    }

    if (parsed.empty())
    {
        plan.enrichment_status = "parse_failed";
        plan.uncertainty_notes += " [model reply not parseable; heuristic steps kept]";
        return;
    }
    plan.steps = std::move(parsed);
    plan.enrichment_status = "model_enriched";
    if (plan.uncertainty_notes.find("[model-enriched]") == std::string::npos)
        plan.uncertainty_notes += " [model-enriched]";
}

} // namespace sovereign::cognitive
