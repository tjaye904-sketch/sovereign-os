#pragma once

#include "../audit/audit_log.hpp"
#include "../capability/capability_gate.hpp"
#include "../governance/governance_service.hpp"

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace sovereign::product
{

enum class PurposeRisk
{
    Low = 0,
    Medium = 1,
    High = 2,
    Critical = 3,
};

struct PurposeEvaluation
{
    bool allowed{false};
    std::string reason;
};

struct PurposeStep
{
    std::string action;
    std::string target;
    std::string detail;
};

struct PurposePack
{
    std::string id;
    std::string purpose_id;
    std::string description;
    PurposeRisk risk{PurposeRisk::Low};
    bool require_headed{false};
    std::vector<std::string> allowed_hosts;
    std::vector<std::string> allowed_apps;
    std::vector<std::string> allowed_tools;
    std::vector<PurposeStep> steps;
};

struct PurposeExecutionStepResult
{
    std::string action;
    std::string target;
    bool allowed{false};
    std::string reason;
};

struct PurposeActionLog
{
    std::string purpose_id;
    std::string action;
    std::string target;
    std::string status;
    std::string detail;
    std::string timestamp;
};

struct PurposeExecutionResult
{
    std::string purpose_id;
    bool allowed{false};
    bool executed{false};
    std::string reason;
    std::vector<PurposeExecutionStepResult> steps;
};

class PurposeRuntime
{
public:
    PurposeRuntime();

    void SetCapabilityGate(sovereign::capability::CapabilityGate* gate) noexcept { gate_ = gate; }
    sovereign::capability::CapabilityGate* Gate() const noexcept { return gate_; }
    void SetGovernanceService(sovereign::governance::GovernanceService* gov) noexcept { governance_ = gov; }
    sovereign::governance::GovernanceService* Governance() const noexcept { return governance_; }
    void SetAuditLog(sovereign::audit::AuditLog* audit) noexcept { audit_ = audit; }
    sovereign::audit::AuditLog* Audit() const noexcept { return audit_; }

    void LoadFromDirectory(const std::filesystem::path& directory);
    void LoadFromFile(const std::filesystem::path& path);
    bool RegisterPack(PurposePack pack);
    std::size_t SeedDefaultLibrary(const std::filesystem::path& directory);
    PurposeEvaluation ValidateAgainstInventory(const std::string& purpose_id, const std::vector<std::string>& allowed_inventory_apps) const;

    const std::vector<PurposePack>& BuiltinPacks() const noexcept;
    std::optional<PurposePack> Find(const std::string& purpose_id) const;
    std::string CurrentPurposeId() const noexcept;
    bool StartPurpose(const std::string& purpose_id, const std::string& summary = {});
    bool SaveSession(const std::filesystem::path& path) const;
    bool LoadSession(const std::filesystem::path& path);
    void LogAction(
        const std::string& purpose_id,
        const std::string& action,
        const std::string& target,
        const std::string& detail,
        const std::string& status = "logged") const;
    std::vector<PurposeActionLog> ActionLog() const;
    PurposeExecutionResult Run(
        const std::string& purpose_id,
        const std::string& target = {},
        const std::string& summary = {}) const;
    PurposeExecutionResult Run(
        const PurposePack& pack,
        const std::string& target = {},
        const std::string& summary = {}) const;
    PurposeEvaluation CanRun(
        const PurposePack& pack,
        const std::string& action,
        const std::string& target,
        const std::string& summary) const;

private:
    static std::string Trim(std::string value);
    static std::string ToLower(std::string value);
    static std::string NormalizeScalar(const std::string& value);
    static std::string CanonicalizeActionName(const std::string& value);
    static std::string ExtractHost(const std::string& target);
    static bool ListContains(const std::vector<std::string>& values, const std::string& value);
    static bool HostAllowed(const PurposePack& pack, const std::string& target);
    static bool AppAllowed(const PurposePack& pack, const std::string& target);
    static bool ParseBool(const std::string& value);
    static PurposeRisk ParseRisk(const std::string& value);

    std::vector<PurposePack> packs_;
    sovereign::capability::CapabilityGate* gate_{nullptr};
    sovereign::governance::GovernanceService* governance_{nullptr};
    sovereign::audit::AuditLog* audit_{nullptr};
    mutable std::string active_purpose_id_;
    mutable std::vector<PurposeActionLog> action_log_;
};

} // namespace sovereign::product
