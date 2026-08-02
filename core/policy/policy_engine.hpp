#pragma once

#include "policy_types.hpp"

#include <string>
#include <vector>

namespace sovereign::policy
{

class PolicyEngine
{
public:
    // roles: role name strings from Identity GetRoles
    Result<PolicyDecision> Evaluate(
        const PrincipalId& principal,
        const std::vector<std::string>& roles,
        const std::string& action) const;
};

} // namespace sovereign::policy
