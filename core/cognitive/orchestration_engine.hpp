#pragma once

#include "cognitive_types.hpp"
#include "governance_service.hpp"
#include "memory_engine.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::cognitive
{

enum class ExecutionStatus : std::uint8_t
{
    Queued = 0,
    InProgress = 1,
    Blocked = 2,
    Completed = 3,
    Failed = 4
};

struct OrchestrationStep
{
    std::string action;
    std::string detail;
    bool requires_governance{false};
    ExecutionStatus status{ExecutionStatus::Queued};
};

struct ExecutionSequence
{
    AnalysisId id{};
    PrincipalId requester{};
    std::string goal;
    std::vector<OrchestrationStep> steps;
    std::array<std::uint8_t, 16> decision_id{};
    ExecutionStatus status{ExecutionStatus::Queued};
    std::uint64_t created_unix_seconds{0};
};

class OrchestrationEngine
{
public:
    explicit OrchestrationEngine(
        sovereign::governance::GovernanceService* governance = nullptr,
        sovereign::memory::MemoryEngine* memory = nullptr);

    Result<ExecutionSequence> BuildSequence(const ExecutionPlan& plan);
    Result<ExecutionSequence> ExecuteApprovedPlan(
        const ExecutionSequence& sequence,
        const sovereign::governance::DecisionId& decision_id);

private:
    static AnalysisId NewId();
    static std::uint64_t NowUnixSeconds();

    sovereign::governance::GovernanceService* governance_;
    sovereign::memory::MemoryEngine* memory_;
};

} // namespace sovereign::cognitive
