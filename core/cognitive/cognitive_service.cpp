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
    sovereign::memory::MemoryEngine* memory)
    : governance_(governance)
    , memory_(memory)
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
        "Heuristic plan without live model; confidence is an estimate. "
        "Governed steps require human approval.";

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

} // namespace sovereign::cognitive
