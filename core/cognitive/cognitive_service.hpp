#pragma once

#include "cognitive_types.hpp"
#include "governance_service.hpp"
#include "memory_engine.hpp"

#include <string>
#include <vector>

namespace sovereign::cognitive
{

// Tightly coupled to Governance: recommendations become pending decisions;
// execution gate checks Approved status.
class CognitiveService
{
public:
    CognitiveService(
        sovereign::governance::GovernanceService& governance,
        sovereign::memory::MemoryEngine* memory = nullptr);

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

    // Creates Pending governance decision — does not execute action.
    Result<DecisionRecommendation> RecommendAction(
        const PrincipalId& requester,
        const std::string& action,
        const std::string& summary,
        const std::string& evidence_ref);

    // Gate: only returns Ok if governance decision is Approved.
    Result<void> AssertApproved(
        const std::array<std::uint8_t, 16>& governance_decision_id) const;

    Result<ReflectionNote> Reflect(
        const PrincipalId& author,
        const std::string& subject,
        const std::string& body);

    // Phase VI: decompose goal into steps + confidence estimate
    Result<ExecutionPlan> BuildPlan(
        const PrincipalId& requester,
        const std::string& goal);

private:
    static AnalysisId NewId();
    static std::uint64_t Now();

    sovereign::governance::GovernanceService& governance_;
    sovereign::memory::MemoryEngine* memory_;
};

} // namespace sovereign::cognitive
