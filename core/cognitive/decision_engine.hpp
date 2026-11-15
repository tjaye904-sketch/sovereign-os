#pragma once

#include "cognitive_types.hpp"
#include "governance_service.hpp"

#include <array>
#include <cstdint>
#include <string>

namespace sovereign::cognitive
{

class DecisionEngine
{
public:
    explicit DecisionEngine(sovereign::governance::GovernanceService* governance = nullptr);

    Result<DecisionRecommendation> RecommendAction(
        const PrincipalId& requester,
        const std::string& action,
        const std::string& summary,
        const std::string& evidence_ref);

    Result<void> AssertApproved(
        const std::array<std::uint8_t, 16>& governance_decision_id) const;

private:
    static AnalysisId NewId();
    static std::uint64_t NowUnixSeconds();

    sovereign::governance::GovernanceService* governance_;
};

} // namespace sovereign::cognitive
