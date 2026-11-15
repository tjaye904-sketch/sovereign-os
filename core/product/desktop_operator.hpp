#pragma once

#include "governance_types.hpp"
#include "principal_id.hpp"

#include <array>
#include <cstdint>
#include <string>

namespace sovereign::product
{

class SovereignHost;

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

private:
    static std::string ToHex(const std::array<std::uint8_t, 16>& bytes);
    static bool FromHex(const std::string& hex, std::array<std::uint8_t, 16>& bytes);
    static bool IsAllowedAction(const std::string& action);

    SovereignHost* host_{nullptr};
};

} // namespace sovereign::product
