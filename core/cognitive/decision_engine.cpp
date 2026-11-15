#include "decision_engine.hpp"

#include <chrono>
#include <random>

namespace sovereign::cognitive
{

AnalysisId DecisionEngine::NewId()
{
    AnalysisId id{};
    std::random_device rd;
    for (auto& byte : id.bytes)
    {
        byte = static_cast<std::uint8_t>(rd());
    }
    return id;
}

std::uint64_t DecisionEngine::NowUnixSeconds()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

DecisionEngine::DecisionEngine(sovereign::governance::GovernanceService* governance)
    : governance_(governance)
{
}

Result<DecisionRecommendation> DecisionEngine::RecommendAction(
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
    if (governance_ == nullptr)
    {
        result.error = CognitiveError::GovernanceRequired;
        return result;
    }

    sovereign::governance::PrincipalId gov_requester{};
    gov_requester.bytes = requester.bytes;

    const auto decision = governance_->RequestDecision(
        gov_requester,
        action,
        summary,
        evidence_ref);
    if (!decision.Ok())
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
    rec.governance_decision_id = decision.value.bytes;
    rec.submitted_to_governance = true;
    rec.created_unix_seconds = NowUnixSeconds();
    result.value = rec;
    return result;
}

Result<void> DecisionEngine::AssertApproved(
    const std::array<std::uint8_t, 16>& governance_decision_id) const
{
    Result<void> result;
    if (governance_ == nullptr)
    {
        result.error = CognitiveError::GovernanceRequired;
        return result;
    }

    sovereign::governance::DecisionId id{};
    id.bytes = governance_decision_id;
    auto got = governance_->Get(id);
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

} // namespace sovereign::cognitive
