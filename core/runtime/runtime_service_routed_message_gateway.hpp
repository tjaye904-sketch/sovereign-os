#pragma once

#include "identity_types.hpp"
#include "runtime_message_bus.hpp"
#include "runtime_service_dispatcher.hpp"

#include <cstdint>
#include <string_view>

namespace sovereign
{

class RuntimeServiceRoutedMessageGateway final
{
public:
    static constexpr std::string_view
        RequestTopic{
            "runtime.service.route.request"};

    static constexpr std::string_view
        ResponseTopic{
            "runtime.service.route.response"};

    RuntimeServiceRoutedMessageGateway(
        RuntimeMessageBus& messaging,
        RuntimeServiceDispatcher& dispatcher);

    RuntimeServiceRoutedMessageGateway(
        const RuntimeServiceRoutedMessageGateway&) =
            delete;

    RuntimeServiceRoutedMessageGateway&
    operator=(
        const RuntimeServiceRoutedMessageGateway&) =
            delete;

    ~RuntimeServiceRoutedMessageGateway();

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
    RuntimeServiceDispatcher& dispatcher_;

    SubscriptionId subscription_id_{
        SubscriptionId::Nil()};

    bool running_{false};

    std::uint64_t request_count_{0U};
    std::uint64_t response_count_{0U};
    std::uint64_t malformed_count_{0U};
};

} // namespace sovereign
