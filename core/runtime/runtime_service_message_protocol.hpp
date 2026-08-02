#pragma once

#include "runtime_service_invocation.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace sovereign
{

struct RuntimeServiceMessageRequest final
{
    std::string service_name{};
    std::string operation{};
    std::string payload{};
};

class RuntimeServiceMessageProtocol final
{
public:
    [[nodiscard]]
    static std::string EncodeRequest(
        const RuntimeServiceMessageRequest& request);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceMessageRequest>
    DecodeRequest(std::string_view payload);

    [[nodiscard]]
    static std::string EncodeResult(
        const RuntimeServiceInvocationResult& result);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceInvocationResult>
    DecodeResult(std::string_view payload);

private:
    RuntimeServiceMessageProtocol() = delete;
};

} // namespace sovereign
