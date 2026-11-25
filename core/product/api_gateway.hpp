#pragma once

#include "desktop_operator.hpp"
#include "governance_types.hpp"
#include "principal_id.hpp"

#include <array>
#include <cstdint>
#include <string>

namespace sovereign::product
{

class SovereignHost;

class ApiGatewayService
{
public:
    explicit ApiGatewayService(SovereignHost& host);

    bool SubmitRequest(
        const PrincipalId& actor,
        const std::string& route,
        const std::string& action,
        const std::string& summary,
        const std::string& evidence_ref,
        std::string& decision_hex);

    bool ApproveDecision(
        const PrincipalId& actor,
        const std::string& decision_hex,
        const std::string& reason);

    bool RejectDecision(
        const PrincipalId& actor,
        const std::string& decision_hex,
        const std::string& reason);

    [[nodiscard]] DesktopOperatorStatus GetDesktopStatus(std::size_t audit_limit = 10) const;

private:
    static std::string ToHex(const std::array<std::uint8_t, 16>& bytes);
    static bool FromHex(const std::string& hex, std::array<std::uint8_t, 16>& bytes);
    static std::string NormalizeRoute(const std::string& route);

    SovereignHost* host_{nullptr};
};

} // namespace sovereign::product
