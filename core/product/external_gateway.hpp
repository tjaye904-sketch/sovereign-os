#pragma once

#include "governance_types.hpp"
#include "principal_id.hpp"

#include <array>
#include <cstdint>
#include <string>

namespace sovereign::product
{

class SovereignHost;

class ExternalInterfaceGateway
{
public:
    explicit ExternalInterfaceGateway(SovereignHost& host);

    bool Request(
        const PrincipalId& actor,
        const std::string& action,
        const std::string& summary,
        const std::string& evidence_ref,
        std::string& decision_hex);

    bool Approve(
        const PrincipalId& actor,
        const std::string& decision_hex,
        const std::string& reason);

    bool Reject(
        const PrincipalId& actor,
        const std::string& decision_hex,
        const std::string& reason);

private:
    static std::string ToHex(const std::array<std::uint8_t, 16>& bytes);
    static bool FromHex(const std::string& hex, std::array<std::uint8_t, 16>& bytes);

    SovereignHost* host_{nullptr};
};

} // namespace sovereign::product
