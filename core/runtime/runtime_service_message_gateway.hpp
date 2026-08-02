#pragma once

#include "identity_types.hpp"
#include "runtime_message_bus.hpp"
#include "runtime_service_invoker.hpp"

#include <cstdint>
#include <string_view>

namespace sovereign
{

class RuntimeServiceMessageGateway final
{
public:
    static constexpr std::string_view
        RequestTopic{
            "runtime.service.invoke.request"};

    static constexpr std::string_view
        ResponseTopic{
            "runtime.service.invoke.response"};

    RuntimeServiceMessageGateway(
        RuntimeMessageBus& messaging,
        RuntimeServiceInvoker& invoker);

    RuntimeServiceMessageGateway(
        const RuntimeServiceMessageGateway&) =
            delete;

    RuntimeServiceMessageGateway&
    operator=(
        const RuntimeServiceMessageGateway&) =
            delete;

    ~RuntimeServiceMessageGateway();

    [[nodiscard]]
    bool Start();

    [[nodiscard]]
    bool Stop() noexcept;

    [[nodiscard]]
    bool Running() const noexcept;

    [[nodiscard]]
    std::uint64_t RequestCount() const noexcept;

    [[nodiscard]]
    std::uint64_t ResponseCount() const noexcept;

    [[nodiscard]]
    std::uint64_t MalformedCount() const noexcept;

private:
    void HandleRequest(const Event& event);

    RuntimeMessageBus& messaging_;
    RuntimeServiceInvoker& invoker_;

    SubscriptionId subscription_id_{
        SubscriptionId::Nil()};

    bool running_{false};

    std::uint64_t request_count_{0U};
    std::uint64_t response_count_{0U};
    std::uint64_t malformed_count_{0U};
};

} // namespace sovereign
