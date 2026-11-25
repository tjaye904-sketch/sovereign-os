#pragma once

#include "governance_types.hpp"
#include "principal_id.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::product
{

class SovereignHost;

struct DesktopOperatorStatus
{
    std::string mode;
    bool has_display{false};
    bool computer_use_enabled{false};
    std::string active_purpose_id;
    std::size_t pending_decisions{0};
    std::size_t evidence_files{0};
    std::vector<std::string> recent_audit_events;
};

struct DesktopDriveReceipt
{
    bool allowed{false};
    bool executed{false};
    std::string op;
    std::string target_app;
    bool headed{false};
    std::string evidence_hash;
    std::string reason;
};

class DesktopOperatorGateway
{
public:
    explicit DesktopOperatorGateway(SovereignHost& host);

    bool RequestAction(
        const PrincipalId& actor,
        const std::string& action,
        const std::string& target,
        const std::string& summary,
        std::string& decision_hex);

    bool Approve(
        const PrincipalId& actor,
        const std::string& decision_hex,
        const std::string& reason);

    bool Reject(
        const PrincipalId& actor,
        const std::string& decision_hex,
        const std::string& reason);

    bool ExecuteApprovedAction(
        const std::string& decision_hex,
        std::string& report);

    [[nodiscard]] DesktopOperatorStatus Status(std::size_t audit_limit = 10) const;
    DesktopDriveReceipt DriveWindow(
        const std::string& op,
        const std::string& target_app,
        const std::string& payload,
        bool require_headed = true);

private:
    static std::string ToHex(const std::array<std::uint8_t, 16>& bytes);
    static bool FromHex(const std::string& hex, std::array<std::uint8_t, 16>& bytes);
    static bool IsAllowedAction(const std::string& action);

    SovereignHost* host_{nullptr};
};

} // namespace sovereign::product
