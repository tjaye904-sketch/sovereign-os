#pragma once

#include "runtime_service_capability_invoker.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace sovereign
{

class RuntimeServiceCapabilityInvocationProtocol final
{
public:
    [[nodiscard]]
    static std::string EncodeRequest(
        const RuntimeServiceCapabilityInvocationRequest&
            request);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceCapabilityInvocationRequest>
    DecodeRequest(std::string_view payload);

    [[nodiscard]]
    static std::string EncodeResult(
        const RuntimeServiceCapabilityInvocationResult&
            result);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceCapabilityInvocationResult>
    DecodeResult(std::string_view payload);

private:
    RuntimeServiceCapabilityInvocationProtocol() =
        delete;
};

} // namespace sovereign
