#pragma once

#include "cognitive_types.hpp"
#include "governance_service.hpp"
#include "memory_engine.hpp"

#include <string>
#include <vector>

namespace sovereign::cognitive
{

class PlanningEngine
{
public:
    explicit PlanningEngine(
        sovereign::governance::GovernanceService* governance = nullptr,
        sovereign::memory::MemoryEngine* memory = nullptr,
        std::string default_model = "openclaw");

    Result<ExecutionPlan> BuildPlan(
        const PrincipalId& requester,
        const std::string& goal);

private:
    static AnalysisId NewId();
    static std::uint64_t NowUnixSeconds();
    static std::string NormalizeGoal(const std::string& value);
    void EnrichPlanWithModel(ExecutionPlan& plan) const;

    sovereign::governance::GovernanceService* governance_;
    sovereign::memory::MemoryEngine* memory_;
    std::string default_model_;
};

} // namespace sovereign::cognitive
