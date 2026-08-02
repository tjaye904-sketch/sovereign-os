#pragma once

#include "runtime_service_dispatcher.hpp"
#include "runtime_service_invocation.hpp"
#include "runtime_service_capability_negotiator.hpp"
#include "runtime_service_capability_invoker.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace sovereign
{

enum class RuntimeServiceEndpointTarget :
    std::uint8_t
{
    Direct = 0U,
    Routed = 1U,
    Capability = 2U,
    CapabilityInvocation = 3U
};

struct RuntimeServiceEndpointRequest final
{
    RuntimeServiceEndpointTarget target{
        RuntimeServiceEndpointTarget::Direct};

    std::string target_name{};
    std::string operation{};
    std::string payload{};
};

struct RuntimeServiceEndpointResult final
{
    RuntimeServiceEndpointTarget target{
        RuntimeServiceEndpointTarget::Direct};

    std::variant<
        RuntimeServiceInvocationResult,
        RuntimeServiceDispatchResult,
        RuntimeServiceCapabilityResult,
        RuntimeServiceCapabilityInvocationResult>
        result{RuntimeServiceInvocationResult{}};

    [[nodiscard]]
    bool Completed() const noexcept;
};

class RuntimeServiceEndpointProtocol final
{
public:
    [[nodiscard]]
    static std::string EncodeRequest(
        const RuntimeServiceEndpointRequest&
            request);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceEndpointRequest>
    DecodeRequest(std::string_view payload);

    [[nodiscard]]
    static std::string EncodeResult(
        const RuntimeServiceEndpointResult&
            result);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceEndpointResult>
    DecodeResult(std::string_view payload);

private:
    RuntimeServiceEndpointProtocol() = delete;
};

} // namespace sovereign
