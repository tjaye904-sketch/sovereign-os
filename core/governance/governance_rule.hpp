#pragma once

#include "governance_role.hpp"

#include <string>

namespace sovereign::governance
{

// Per-action rule (from Claude design, kept file-durable in our service).
struct GovernanceRule
{
    std::string action;
    bool allow_auto_approval{false};
    GovernanceRole minimum_auto_approve_role{GovernanceRole::Owner};
    GovernanceRole minimum_approval_authority_role{GovernanceRole::Owner};
};

[[nodiscard]] inline GovernanceRule DefaultGovernanceRule(std::string action)
{
    return GovernanceRule{
        .action = std::move(action),
        .allow_auto_approval = false,
        .minimum_auto_approve_role = GovernanceRole::Owner,
        .minimum_approval_authority_role = GovernanceRole::Owner};
}

} // namespace sovereign::governance
