#include "cognitive_service.hpp"

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
    , models_(models)
    , default_model_(std::move(default_model))
{
}

Result<CuriosityProposal> CognitiveService::ProposeCuriosity(
    const PrincipalId& requester,
    const std::string& topic,
    const std::vector<std::string>& questions)
{
    Result<CuriosityProposal> result;
    if (topic.empty())
    {
        result.error = CognitiveError::InvalidArgument;
        return result;
    }
    CuriosityProposal p;
    p.id = NewId();
    p.requester = requester;
    p.topic = topic;
    p.questions = questions;
    p.created_unix_seconds = Now();
    result.value = p;
    return result;
}

Result<ReasoningResult> CognitiveService::Reason(
    const PrincipalId& requester,
    const std::string& question,
    const std::vector<std::string>& points_for,
    const std::vector<std::string>& points_against,
    const std::string& summary)
{
    Result<ReasoningResult> result;
    if (question.empty())
    {
        result.error = CognitiveError::InvalidArgument;
        return result;
    }
    ReasoningResult r;
    r.id = NewId();
    r.requester = requester;
    r.question = question;
    r.points_for = points_for;
    r.points_against = points_against;
    r.summary = summary;
    r.created_unix_seconds = Now();
    result.value = r;
    return result;
}

Result<DecisionRecommendation> CognitiveService::RecommendAction(
    const PrincipalId& requester,
    const std::string& action,
    const std::string& summary,
    const std::string& evidence_ref)
{
    Result<DecisionRecommendation> result;
    if (action.empty())
    {
        result.error = CognitiveError::InvalidArgument;
        return result;
    }

    sovereign::governance::PrincipalId gov_requester{};
    gov_requester.bytes = requester.bytes;

    auto dec = governance_.RequestDecision(
        gov_requester,
        action,
        summary,
        evidence_ref);
    if (!dec.Ok())
    {
        result.error = CognitiveError::StorageFailure;
        return result;
    }

    DecisionRecommendation rec;
    rec.id = NewId();
    rec.requester = requester;
    rec.action = action;
    rec.summary = summary;
    rec.evidence_ref = evidence_ref;
    rec.governance_decision_id = dec.value.bytes;
    rec.submitted_to_governance = true;
    rec.created_unix_seconds = Now();
    result.value = rec;
    return result;
}

Result<void> CognitiveService::AssertApproved(
    const std::array<std::uint8_t, 16>& governance_decision_id) const
{
    Result<void> result;
    sovereign::governance::DecisionId id{};
    id.bytes = governance_decision_id;
    auto got = governance_.Get(id);
    if (!got.Ok())
    {
        result.error = CognitiveError::GovernanceRequired;
        return result;
    }
    if (got.value.status != sovereign::governance::DecisionStatus::Approved)
    {
        result.error = CognitiveError::NotApproved;
        return result;
    }
    return result;
}

Result<ReflectionNote> CognitiveService::Reflect(
    const PrincipalId& author,
    const std::string& subject,
    const std::string& body)
{
    Result<ReflectionNote> result;
    if (subject.empty() || body.empty())
    {
        result.error = CognitiveError::InvalidArgument;
        return result;
    }
    ReflectionNote note;
    note.id = NewId();
    note.author = author;
    note.subject = subject;
    note.body = body;
    note.created_unix_seconds = Now();

    if (memory_ != nullptr)
    {
        sovereign::memory::MemoryRecord rec;
        rec.author = author;
        rec.tags = {"reflection", subject};
        rec.body = body;
        auto app = memory_->Append(rec);
        if (!app.Ok())
        {
            result.error = CognitiveError::StorageFailure;
            return result;
        }
    }

    result.value = note;
    return result;
}

Result<ExecutionPlan> CognitiveService::BuildPlan(
    const PrincipalId& requester,
    const std::string& goal)
{
    Result<ExecutionPlan> result;
    if (goal.empty())
    {
        result.error = CognitiveError::InvalidArgument;
        return result;
    }
    ExecutionPlan plan;
    plan.id = NewId();
    plan.requester = requester;
    plan.goal = goal;
    plan.created_unix_seconds = Now();

    // Deterministic decomposition heuristics (local, no model required)
    PlanStep s1;
    s1.action = "inspect";
    s1.detail = "Review current system status and relevant memory";
    plan.steps.push_back(s1);

    PlanStep s2;
    s2.action = "policy-dry-run";
    s2.detail = "Dry-run policy for candidate actions related to: " + goal;
    plan.steps.push_back(s2);

    PlanStep s3;
    s3.action = "recommend";
    s3.detail = "Submit governed recommendation for goal";
    s3.requires_governance = true;
    plan.steps.push_back(s3);

    PlanStep s4;
    s4.action = "execute";
    s4.detail = "Execute only after governance approval";
    s4.requires_governance = true;
    plan.steps.push_back(s4);

    // Confidence: higher when goal is short/specific; lower when vague
    const double len_factor = goal.size() < 20 ? 0.55 : (goal.size() < 80 ? 0.72 : 0.65);
    plan.confidence = len_factor;
    plan.uncertainty_notes =
        "Heuristic plan; governed steps require human approval.";

    EnrichPlanWithModel(plan);

    if (memory_)
    {
        sovereign::memory::MemoryRecord rec;
        rec.author = requester;
        rec.tags = {"cognitive", "plan"};
        rec.body = "plan:" + goal + " conf=" + std::to_string(plan.confidence);
        rec.importance = 0.7;
        static_cast<void>(memory_->Append(rec));
    }
    result.value = std::move(plan);
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
            if (candidates.size() >= 4)
                break;
        }
    }
    if (candidates.empty())
        candidates.push_back("openclaw");

    sovereign::model_router::ChatRequest req;
    req.temperature = 0.2;
    req.num_predict = 500;
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
