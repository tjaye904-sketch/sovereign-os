#include "planning_engine.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <random>

namespace sovereign::cognitive
{

AnalysisId PlanningEngine::NewId()
{
    AnalysisId id{};
    std::random_device rd;
    for (auto& byte : id.bytes)
    {
        byte = static_cast<std::uint8_t>(rd());
    }
    return id;
}

std::uint64_t PlanningEngine::NowUnixSeconds()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

std::string PlanningEngine::NormalizeGoal(const std::string& value)
{
    std::string out = value;
    const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    out.erase(out.begin(), std::find_if(out.begin(), out.end(), not_space));
    out.erase(std::find_if(out.rbegin(), out.rend(), not_space).base(), out.end());
    return out;
}

PlanningEngine::PlanningEngine(
    sovereign::governance::GovernanceService* governance,
    sovereign::memory::MemoryEngine* memory,
    std::string default_model)
    : governance_(governance)
    , memory_(memory)
    , default_model_(std::move(default_model))
{
}

// Heuristic only — never claim model_enriched from a model *name*.
// Live enrichment is CognitiveService::EnrichPlanWithModel (router required).
void PlanningEngine::EnrichPlanWithModel(ExecutionPlan& plan) const
{
    plan.enrichment_status = "heuristic";
    if (plan.goal.size() > 120)
    {
        plan.confidence = 0.58;
        plan.uncertainty_notes =
            "Goal is broad; consider narrowing scope before execution.";
        return;
    }
    plan.confidence = 0.74;
    plan.uncertainty_notes =
        "Heuristic plan; governed steps require human approval.";
    (void)default_model_; // selection happens in CognitiveService when router is live
}

Result<ExecutionPlan> PlanningEngine::BuildPlan(
    const PrincipalId& requester,
    const std::string& goal)
{
    Result<ExecutionPlan> result;
    const std::string normalized = NormalizeGoal(goal);
    if (normalized.empty())
    {
        result.error = CognitiveError::InvalidArgument;
        return result;
    }

    ExecutionPlan plan;
    plan.id = NewId();
    plan.requester = requester;
    plan.goal = normalized;
    plan.created_unix_seconds = NowUnixSeconds();

    PlanStep inspect;
    inspect.action = "inspect";
    inspect.detail =
        "Review current state, constraints, and any relevant memory for: " + normalized;
    plan.steps.push_back(inspect);

    PlanStep policy;
    policy.action = "policy-dry-run";
    policy.detail =
        "Evaluate expected actions against governance and policy before execution.";
    plan.steps.push_back(policy);

    PlanStep recommend;
    recommend.action = "recommend";
    recommend.detail = "Submit a governance decision for the proposed action path.";
    recommend.requires_governance = true;
    plan.steps.push_back(recommend);

    PlanStep execute;
    execute.action = "execute";
    execute.detail =
        "Execute only after the decision is approved and the action remains in scope.";
    execute.requires_governance = true;
    plan.steps.push_back(execute);

    EnrichPlanWithModel(plan);

    if (memory_ != nullptr)
    {
        sovereign::memory::MemoryRecord rec;
        rec.author = requester;
        rec.tags = {"cognitive", "planning"};
        rec.body = "goal:" + plan.goal + "|confidence=" + std::to_string(plan.confidence);
        rec.importance = 0.75;
        rec.created_unix_seconds = plan.created_unix_seconds;
        static_cast<void>(memory_->Append(rec));
    }

    if (governance_ != nullptr && !plan.enrichment_status.empty())
    {
        plan.uncertainty_notes += " [governance-gated]";
    }

    result.value = plan;
    return result;
}

} // namespace sovereign::cognitive
