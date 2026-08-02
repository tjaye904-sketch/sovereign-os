#pragma once

#include "governance_rule.hpp"
#include "governance_types.hpp"

#include <filesystem>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace sovereign::governance
{

// File-backed governance with Claude-derived rule/role/override model.
// Durable under data_root/governance. Caller supplies authenticated principals.
class GovernanceService
{
public:
    GovernanceError Open(const std::filesystem::path& directory);
    void Close() noexcept;

    void SetRule(GovernanceRule rule);
    [[nodiscard]] GovernanceRule RuleFor(const std::string& action) const;

    // Creates Pending decision, or auto-approves if rule allows and role suffices.
    Result<DecisionId> RequestDecision(
        const PrincipalId& requester,
        GovernanceRole requester_role,
        const std::string& action,
        const std::string& summary,
        const std::string& evidence_ref);

    // Backward-compatible: treats requester as Operator (no auto unless rule allows operator).
    Result<DecisionId> RequestDecision(
        const PrincipalId& requester,
        const std::string& action,
        const std::string& summary,
        const std::string& evidence_ref);

    Result<std::vector<Decision>> ListPending() const;
    Result<Decision> Get(const DecisionId& id) const;

    Result<void> Approve(
        const PrincipalId& actor,
        GovernanceRole actor_role,
        const DecisionId& id,
        const std::string& reason);

    Result<void> Reject(
        const PrincipalId& actor,
        GovernanceRole actor_role,
        const DecisionId& id,
        const std::string& reason);

    // Legacy bool owner_is_owner_role API
    Result<void> Approve(
        const PrincipalId& owner,
        bool owner_is_owner_role,
        const DecisionId& id,
        const std::string& reason);

    Result<void> Reject(
        const PrincipalId& owner,
        bool owner_is_owner_role,
        const DecisionId& id,
        const std::string& reason);

    // Owner-only emergency override (always recorded is_override=true).
    Result<void> Override(
        const PrincipalId& owner,
        GovernanceRole actor_role,
        const DecisionId& id,
        const std::string& reason);

private:
    GovernanceError Load();
    GovernanceError Save() const;
    GovernanceError LoadRules();
    GovernanceError SaveRules() const;
    static DecisionId NewId();
    static std::uint64_t Now();

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path directory_;
    std::vector<Decision> decisions_;
    std::unordered_map<std::string, GovernanceRule> rules_;
};

} // namespace sovereign::governance
