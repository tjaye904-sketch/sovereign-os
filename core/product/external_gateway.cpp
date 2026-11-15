#include "external_gateway.hpp"

#include "sovereign_host.hpp"

namespace sovereign::product
{

ExternalInterfaceGateway::ExternalInterfaceGateway(SovereignHost& host)
    : host_(&host)
{
}

std::string ExternalInterfaceGateway::ToHex(const std::array<std::uint8_t, 16>& bytes)
{
    static constexpr char kHex[] = "0123456789abcdef";
    std::string out(32, '0');
    for (std::size_t i = 0; i < 16; ++i)
    {
        out[i * 2] = kHex[(bytes[i] >> 4) & 0xF];
        out[i * 2 + 1] = kHex[bytes[i] & 0xF];
    }
    return out;
}

bool ExternalInterfaceGateway::FromHex(const std::string& hex, std::array<std::uint8_t, 16>& bytes)
{
    if (hex.size() != 32)
    {
        return false;
    }

    auto to_nibble = [](char ch) -> int {
        if (ch >= '0' && ch <= '9')
            return ch - '0';
        if (ch >= 'a' && ch <= 'f')
            return ch - 'a' + 10;
        if (ch >= 'A' && ch <= 'F')
            return ch - 'A' + 10;
        return -1;
    };

    for (std::size_t i = 0; i < 16; ++i)
    {
        const int hi = to_nibble(hex[i * 2]);
        const int lo = to_nibble(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0)
        {
            return false;
        }
        bytes[i] = static_cast<std::uint8_t>((hi << 4) | lo);
    }
    return true;
}

bool ExternalInterfaceGateway::Request(
    const PrincipalId& actor,
    const std::string& action,
    const std::string& summary,
    const std::string& evidence_ref,
    std::string& decision_hex)
{
    if (host_ == nullptr)
    {
        return false;
    }
    auto request = host_->Governance().RequestDecision(actor, action, summary, evidence_ref);
    if (!request.Ok())
    {
        return false;
    }
    decision_hex = ToHex(request.value.bytes);
    return true;
}

bool ExternalInterfaceGateway::Approve(
    const PrincipalId& actor,
    const std::string& decision_hex,
    const std::string& reason)
{
    if (host_ == nullptr)
    {
        return false;
    }

    governance::DecisionId id{};
    if (!FromHex(decision_hex, id.bytes))
    {
        return false;
    }

    const auto result = host_->Governance().Approve(actor, true, id, reason);
    return result.Ok();
}

bool ExternalInterfaceGateway::Reject(
    const PrincipalId& actor,
    const std::string& decision_hex,
    const std::string& reason)
{
    if (host_ == nullptr)
    {
        return false;
    }

    governance::DecisionId id{};
    if (!FromHex(decision_hex, id.bytes))
    {
        return false;
    }

    const auto result = host_->Governance().Reject(actor, true, id, reason);
    return result.Ok();
}

} // namespace sovereign::product
