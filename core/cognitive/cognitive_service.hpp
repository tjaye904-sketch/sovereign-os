#pragma once

#include "cognitive_types.hpp"
#include "curiosity_engine.hpp"
#include "decision_engine.hpp"
#include "governance_service.hpp"
#include "memory_engine.hpp"
#include "model_router.hpp"
#include "orchestration_engine.hpp"
#include "planning_engine.hpp"
#include "reasoning_engine.hpp"
#include "reflection_engine.hpp"

#include <string>
#include <vector>

namespace sovereign::cognitive
{

class CognitiveService
{
public:
    CognitiveService(
        sovereign::governance::GovernanceService& governance,
        sovereign::memory::MemoryEngine* memory = nullptr,
        sovereign::model_router::ModelRouter* models = nullptr,
        std::string default_model = "openclaw");

    Result<CuriosityProposal> ProposeCuriosity(
        const PrincipalId& requester,
        const std::string& topic,
        const std::vector<std::string>& questions);

    Result<ReasoningResult> Reason(
        const PrincipalId& requester,
        const std::string& question,
        const std::vector<std::string>& points_for,
        const std::vector<std::string>& points_against,
        const std::string& summary);

    Result<DecisionRecommendation> RecommendAction(
        const PrincipalId& requester,
        const std::string& action,
        const std::string& summary,
        const std::string& evidence_ref);

    Result<void> AssertApproved(
        const std::array<std::uint8_t, 16>& governance_decision_id) const;

    Result<ReflectionNote> Reflect(
        const PrincipalId& author,
        const std::string& subject,
        const std::string& body);

    Result<ExecutionPlan> BuildPlan(
        const PrincipalId& requester,
        const std::string& goal);

private:
    static AnalysisId NewId();
    static std::uint64_t Now();
    void EnrichPlanWithModel(ExecutionPlan& plan) const;

    sovereign::governance::GovernanceService& governance_;
    sovereign::memory::MemoryEngine* memory_;
    CuriosityEngine curiosity_engine_;
    ReasoningEngine reasoning_engine_;
    DecisionEngine decision_engine_;
    ReflectionEngine reflection_engine_;
    PlanningEngine planning_engine_;
    OrchestrationEngine orchestration_engine_;
    sovereign::model_router::ModelRouter* models_;
    std::string default_model_;
};

} // namespace sovereign::cognitive
