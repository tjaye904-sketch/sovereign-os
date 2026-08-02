#include "policy_engine.hpp"

namespace sovereign::policy
{

Result<PolicyDecision> PolicyEngine::Evaluate(
    const PrincipalId& /*principal*/,
    const std::vector<std::string>& roles,
    const std::string& action) const
{
    Result<PolicyDecision> result;
    if (action.empty())
    {
        result.error = PolicyError::InvalidArgument;
        return result;
    }

    bool is_owner = false;
    bool is_operator = false;
    bool is_readonly = false;
    for (const auto& r : roles)
    {
        if (r == "owner") is_owner = true;
        if (r == "operator") is_operator = true;
        if (r == "readonly") is_readonly = true;
    }

    if (is_owner)
    {
        result.value = PolicyDecision{PolicyEffect::Allow, "owner role"};
        return result;
    }

    if (is_readonly)
    {
        result.value = PolicyDecision{PolicyEffect::Deny, "readonly role"};
        return result;
    }

    if (is_operator)
    {
        // Operators: allow non-destructive local ops; deny deploy/delete money-like
        if (action == "deploy" || action == "delete" || action == "transfer")
        {
            result.value =
                PolicyDecision{PolicyEffect::Deny, "operator cannot " + action};
            return result;
        }
        result.value = PolicyDecision{PolicyEffect::Allow, "operator role"};
        return result;
    }

    result.value = PolicyDecision{PolicyEffect::Deny, "no matching role"};
    return result;
}

} // namespace sovereign::policy
