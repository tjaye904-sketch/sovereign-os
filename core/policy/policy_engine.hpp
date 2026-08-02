#pragma once

#include "policy_types.hpp"

#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

namespace sovereign::policy
{

class PolicyEngine
{
public:
    // Optional durable versioned rules under data_root/policy/
    PolicyError Open(const std::filesystem::path& directory);
    void Close() noexcept;

    Result<PolicyDecision> Evaluate(
        const PrincipalId& principal,
        const std::vector<std::string>& roles,
        const std::string& action) const;

    // Phase IV: same as Evaluate but marks dry_run and never mutates store
    Result<PolicyDecision> DryRun(
        const PrincipalId& principal,
        const std::vector<std::string>& roles,
        const std::string& action) const;

    std::uint64_t Version() const;
    Result<void> SetRule(const PolicyRule& rule); // bumps version
    Result<std::vector<PolicyRule>> ListRules() const;

private:
    Result<PolicyDecision> EvaluateImpl(
        const std::vector<std::string>& roles,
        const std::string& action,
        bool dry_run) const;
    PolicyError Load();
    PolicyError Save() const;
    static int RoleRank(const std::string& role);

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path directory_;
    std::uint64_t version_{1};
    std::vector<PolicyRule> rules_;
};

} // namespace sovereign::policy
