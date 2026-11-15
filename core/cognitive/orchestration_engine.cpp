#include "orchestration_engine.hpp"

#include <chrono>
#include <random>

namespace sovereign::cognitive
{

AnalysisId OrchestrationEngine::NewId()
{
    AnalysisId id{};
    std::random_device rd;
    for (auto& byte : id.bytes)
    {
        byte = static_cast<std::uint8_t>(rd());
    }
    return id;
}

std::uint64_t OrchestrationEngine::NowUnixSeconds()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

OrchestrationEngine::OrchestrationEngine(
    sovereign::governance::GovernanceService* governance,
    sovereign::memory::MemoryEngine* memory)
    : governance_(governance)
    , memory_(memory)
{
}

Result<ExecutionSequence> OrchestrationEngine::BuildSequence(const ExecutionPlan& plan)
{
    Result<ExecutionSequence> result;
    if (plan.goal.empty())
    {
        result.error = CognitiveError::InvalidArgument;
        return result;
    }

    ExecutionSequence seq;
    seq.id = NewId();
    seq.requester = plan.requester;
    seq.goal = plan.goal;
    seq.created_unix_seconds = NowUnixSeconds();
    seq.status = ExecutionStatus::Queued;

    for (const auto& step : plan.steps)
    {
        OrchestrationStep item;
        item.action = step.action;
        item.detail = step.detail;
        item.requires_governance = step.requires_governance;
        item.status = ExecutionStatus::Queued;
        seq.steps.push_back(item);
    }

    if (governance_ != nullptr)
    {
        const auto allowed = governance_->RequestDecision(
            sovereign::governance::PrincipalId{plan.requester.bytes},
            "execute-plan",
            "Sequence execution for: " + plan.goal,
            "plan://" + plan.goal);
        if (allowed.Ok())
        {
            seq.decision_id = allowed.value.bytes;
        }
    }

    if (memory_ != nullptr)
    {
        sovereign::memory::MemoryRecord rec;
        rec.author = plan.requester;
        rec.tags = {"cognitive", "orchestration"};
        rec.body = "goal:" + seq.goal + "|decision_id=" + std::to_string(seq.decision_id[0]);
        rec.importance = 0.7;
        rec.created_unix_seconds = seq.created_unix_seconds;
        static_cast<void>(memory_->Append(rec));
    }

    result.value = seq;
    return result;
}

Result<ExecutionSequence> OrchestrationEngine::ExecuteApprovedPlan(
    const ExecutionSequence& sequence,
    const sovereign::governance::DecisionId& decision_id)
{
    Result<ExecutionSequence> result;
    if (sequence.steps.empty())
    {
        result.error = CognitiveError::InvalidArgument;
        return result;
    }
    if (governance_ != nullptr)
    {
        const auto got = governance_->Get(decision_id);
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
    }

    ExecutionSequence executed = sequence;
    executed.status = ExecutionStatus::Completed;
    for (auto& step : executed.steps)
    {
        step.status = ExecutionStatus::Completed;
    }
    result.value = executed;
    return result;
}

} // namespace sovereign::cognitive
