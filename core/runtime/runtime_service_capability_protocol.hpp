#pragma once

#include "runtime_service_capability_negotiator.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace sovereign
{

class RuntimeServiceCapabilityProtocol final
{
public:
    [[nodiscard]]
    static std::string EncodeRequest(
        const RuntimeServiceCapabilityRequest&
            request);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceCapabilityRequest>
    DecodeRequest(std::string_view payload);

    [[nodiscard]]
    static std::string EncodeResult(
        const RuntimeServiceCapabilityResult&
            result);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceCapabilityResult>
    DecodeResult(std::string_view payload);

private:
    RuntimeServiceCapabilityProtocol() = delete;
};

} // namespace sovereign
