#pragma once

#include "runtime_service_capability_invocation_policy.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace sovereign
{

enum class
RuntimeServiceCapabilityInvocationPolicyCommand :
    std::uint8_t
{
    DenyCapability = 0U,
    DenyOperation,
    DenyProvider,
    Query
};

struct
RuntimeServiceCapabilityInvocationPolicyProtocolRequest
    final
{
    RuntimeServiceCapabilityInvocationPolicyCommand
        command{
            RuntimeServiceCapabilityInvocationPolicyCommand::
                Query};

    std::string capability{};
    std::string operation{};
    std::string provider_service_name{};
};

enum class
RuntimeServiceCapabilityInvocationPolicyProtocolStatus :
    std::uint8_t
{
    NoResult = 0U,
    Accepted,
    InvalidPayload,
    UnknownCommand
};

struct
RuntimeServiceCapabilityInvocationPolicyProtocolResult
    final
{
    RuntimeServiceCapabilityInvocationPolicyProtocolStatus
        status{
            RuntimeServiceCapabilityInvocationPolicyProtocolStatus::
                NoResult};

    std::size_t denied_capability_count{0U};
    std::size_t denied_operation_count{0U};
    std::size_t denied_provider_count{0U};

    std::string diagnostic{};

    [[nodiscard]]
    bool Accepted() const noexcept
    {
        return
            status ==
            RuntimeServiceCapabilityInvocationPolicyProtocolStatus::
                Accepted;
    }
};

class
RuntimeServiceCapabilityInvocationPolicyProtocol final
{
public:
    /*
     * Existing RT-113 decision serialization contract.
     */
    [[nodiscard]]
    static std::string EncodeDecision(
        const
        RuntimeServiceCapabilityInvocationPolicyDecision&
            decision);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceCapabilityInvocationPolicyDecision>
    DecodeDecision(std::string_view payload);

    /*
     * RT-117 policy-control contract.
     */
    [[nodiscard]]
    static std::string EncodeRequest(
        const
        RuntimeServiceCapabilityInvocationPolicyProtocolRequest&
            request);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceCapabilityInvocationPolicyProtocolRequest>
    DecodeRequest(std::string_view payload);

    [[nodiscard]]
    static std::string EncodeResult(
        const
        RuntimeServiceCapabilityInvocationPolicyProtocolResult&
            result);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceCapabilityInvocationPolicyProtocolResult>
    DecodeResult(std::string_view payload);

private:
    RuntimeServiceCapabilityInvocationPolicyProtocol() =
        delete;
};

} // namespace sovereign
