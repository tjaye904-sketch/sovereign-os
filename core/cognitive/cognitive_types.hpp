#pragma once

#include "principal_id.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::cognitive
{

using ::sovereign::PrincipalId;

struct AnalysisId
{
    std::array<std::uint8_t, 16> bytes{};
};

struct CuriosityProposal
{
    AnalysisId id{};
    PrincipalId requester{};
    std::string topic;
    std::vector<std::string> questions;
    std::uint64_t created_unix_seconds{0};
};

struct ReasoningResult
{
    AnalysisId id{};
    PrincipalId requester{};
    std::string question;
    std::vector<std::string> points_for;
    std::vector<std::string> points_against;
    std::string summary;
    std::uint64_t created_unix_seconds{0};
};

struct DecisionRecommendation
{
    AnalysisId id{};
    PrincipalId requester{};
    std::string action;
    std::string summary;
    std::string evidence_ref;
    // Filled when submitted to Governance
    std::array<std::uint8_t, 16> governance_decision_id{};
    bool submitted_to_governance{false};
    std::uint64_t created_unix_seconds{0};
};

struct ReflectionNote
{
    AnalysisId id{};
    PrincipalId author{};
    std::string subject;
    std::string body;
    std::uint64_t created_unix_seconds{0};
};

// Phase VI — multi-step plan with confidence
struct PlanStep
{
    std::string action;
    std::string detail;
    bool requires_governance{false};
};

struct ExecutionPlan
{
    AnalysisId id{};
    PrincipalId requester{};
    std::string goal;
    std::vector<PlanStep> steps;
    double confidence{0.0}; // 0..1
    std::string uncertainty_notes;
    // heuristic | model_enriched | model_unavailable | parse_failed | no_router
    std::string enrichment_status{"heuristic"};
    std::uint64_t created_unix_seconds{0};
};

enum class CognitiveError : std::uint8_t
{
    Ok = 0,
    InvalidArgument,
    Unauthorized,
    GovernanceRequired,
    NotApproved,
    StorageFailure
};

template <typename T>
struct Result
{
    CognitiveError error{CognitiveError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == CognitiveError::Ok;
    }
};

template <>
struct Result<void>
{
    CognitiveError error{CognitiveError::Ok};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == CognitiveError::Ok;
    }
};

} // namespace sovereign::cognitive
