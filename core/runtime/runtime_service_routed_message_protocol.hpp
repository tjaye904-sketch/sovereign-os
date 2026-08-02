#pragma once

#include "runtime_service_dispatcher.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace sovereign
{

struct RuntimeServiceRoutedMessageRequest final
{
    std::string route_name{};
    std::string operation{};
    std::string payload{};
};

class RuntimeServiceRoutedMessageProtocol final
{
public:
    [[nodiscard]]
    static std::string EncodeRequest(
        const RuntimeServiceRoutedMessageRequest&
            request);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceRoutedMessageRequest>
    DecodeRequest(std::string_view payload);

    [[nodiscard]]
    static std::string EncodeResult(
        const RuntimeServiceDispatchResult& result);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceDispatchResult>
    DecodeResult(std::string_view payload);

private:
    RuntimeServiceRoutedMessageProtocol() = delete;
};

} // namespace sovereign
