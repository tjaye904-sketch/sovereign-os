#include "policy_engine.hpp"

#include <algorithm>
#include <fstream>

namespace sovereign::policy
{

int PolicyEngine::RoleRank(const std::string& role)
{
    if (role == "owner")
        return 100;
    if (role == "operator")
        return 50;
    if (role == "service")
        return 30;
    if (role == "readonly")
        return 10;
    return 0;
}

PolicyError PolicyEngine::Open(const std::filesystem::path& directory)
{
    std::lock_guard lock(mutex_);
    directory_ = directory;
    std::error_code ec;
    std::filesystem::create_directories(directory_, ec);
    if (ec)
        return PolicyError::StorageFailure;
    open_ = true;
    return Load();
}

void PolicyEngine::Close() noexcept
{
    std::lock_guard lock(mutex_);
    open_ = false;
    rules_.clear();
}

PolicyError PolicyEngine::Load()
{
    rules_.clear();
    version_ = 1;
    const auto path = directory_ / "rules.v1";
    std::ifstream in(path);
    if (!in)
    {
        // defaults
        rules_.push_back(PolicyRule{"deploy", "owner", PolicyEffect::Deny});
        rules_.push_back(PolicyRule{"delete", "owner", PolicyEffect::Deny});
        rules_.push_back(PolicyRule{"transfer", "owner", PolicyEffect::Deny});
        return PolicyError::Ok;
    }
    std::string line;
    if (std::getline(in, line))
    {
        if (line.rfind("version=", 0) == 0)
            version_ = std::stoull(line.substr(8));
    }
    while (std::getline(in, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        // action|min_role|effect
        auto p1 = line.find('|');
        auto p2 = line.find('|', p1 == std::string::npos ? 0 : p1 + 1);
        if (p1 == std::string::npos || p2 == std::string::npos)
            continue;
        PolicyRule r;
        r.action = line.substr(0, p1);
        r.min_role = line.substr(p1 + 1, p2 - p1 - 1);
        r.effect = (line.substr(p2 + 1) == "allow") ? PolicyEffect::Allow
                                                    : PolicyEffect::Deny;
        rules_.push_back(std::move(r));
    }
    return PolicyError::Ok;
}

PolicyError PolicyEngine::Save() const
{
    if (!open_)
        return PolicyError::StorageFailure;
    const auto path = directory_ / "rules.v1";
    std::ofstream out(path, std::ios::trunc);
    if (!out)
        return PolicyError::StorageFailure;
    out << "version=" << version_ << '\n';
    for (const auto& r : rules_)
        out << r.action << '|' << r.min_role << '|'
            << (r.effect == PolicyEffect::Allow ? "allow" : "deny") << '\n';
    return out ? PolicyError::Ok : PolicyError::StorageFailure;
}

std::uint64_t PolicyEngine::Version() const
{
    std::lock_guard lock(mutex_);
    return version_;
}

Result<void> PolicyEngine::SetRule(const PolicyRule& rule)
{
    Result<void> result;
    std::lock_guard lock(mutex_);
    if (rule.action.empty() || rule.min_role.empty())
    {
        result.error = PolicyError::InvalidArgument;
        return result;
    }
    auto it = std::find_if(rules_.begin(), rules_.end(),
                           [&](const PolicyRule& x) {
                               return x.action == rule.action;
                           });
    if (it != rules_.end())
        *it = rule;
    else
        rules_.push_back(rule);
    ++version_;
    if (Save() != PolicyError::Ok)
        result.error = PolicyError::StorageFailure;
    return result;
}

Result<std::vector<PolicyRule>> PolicyEngine::ListRules() const
{
    Result<std::vector<PolicyRule>> result;
    std::lock_guard lock(mutex_);
    result.value = rules_;
    return result;
}

Result<PolicyDecision> PolicyEngine::EvaluateImpl(
    const std::vector<std::string>& roles,
    const std::string& action,
    bool dry_run) const
{
    Result<PolicyDecision> result;
    if (action.empty())
    {
        result.error = PolicyError::InvalidArgument;
        return result;
    }

    int best = 0;
    for (const auto& r : roles)
        best = std::max(best, RoleRank(r));

    // explicit rules first
    for (const auto& rule : rules_)
    {
        if (rule.action != action)
            continue;
        const int need = RoleRank(rule.min_role);
        if (best >= need && rule.effect == PolicyEffect::Allow)
        {
            result.value = PolicyDecision{PolicyEffect::Allow,
                                          "rule allow " + action, version_,
                                          dry_run};
            return result;
        }
        if (best < need || rule.effect == PolicyEffect::Deny)
        {
            // Deny rules for restricted actions unless rank meets owner-class
            if (rule.effect == PolicyEffect::Deny && best < RoleRank("owner"))
            {
                result.value = PolicyDecision{
                    PolicyEffect::Deny, "rule deny " + action + " needs " + rule.min_role,
                    version_, dry_run};
                return result;
            }
        }
    }

    // built-in role ladder
    if (best >= RoleRank("owner"))
    {
        result.value =
            PolicyDecision{PolicyEffect::Allow, "owner role", version_, dry_run};
        return result;
    }
    if (best >= RoleRank("operator"))
    {
        if (action == "deploy" || action == "delete" || action == "transfer")
        {
            result.value = PolicyDecision{PolicyEffect::Deny,
                                          "operator cannot " + action, version_,
                                          dry_run};
            return result;
        }
        result.value = PolicyDecision{PolicyEffect::Allow, "operator role",
                                      version_, dry_run};
        return result;
    }
    if (best >= RoleRank("service"))
    {
        if (action == "read" || action == "status" || action == "memory-add")
        {
            result.value = PolicyDecision{PolicyEffect::Allow, "service role",
                                          version_, dry_run};
            return result;
        }
        result.value = PolicyDecision{PolicyEffect::Deny,
                                      "service limited", version_, dry_run};
        return result;
    }
    if (best >= RoleRank("readonly"))
    {
        result.value =
            PolicyDecision{PolicyEffect::Deny, "readonly role", version_, dry_run};
        return result;
    }
    result.value =
        PolicyDecision{PolicyEffect::Deny, "no matching role", version_, dry_run};
    return result;
}

Result<PolicyDecision> PolicyEngine::Evaluate(
    const PrincipalId& /*principal*/,
    const std::vector<std::string>& roles,
    const std::string& action) const
{
    std::lock_guard lock(mutex_);
    return EvaluateImpl(roles, action, false);
}

Result<PolicyDecision> PolicyEngine::DryRun(
    const PrincipalId& /*principal*/,
    const std::vector<std::string>& roles,
    const std::string& action) const
{
    std::lock_guard lock(mutex_);
    return EvaluateImpl(roles, action, true);
}

} // namespace sovereign::policy
