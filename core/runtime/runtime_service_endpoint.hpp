#pragma once

#include "identity_types.hpp"
#include "runtime_message_bus.hpp"
#include "runtime_service_dispatcher.hpp"
#include "runtime_service_endpoint_protocol.hpp"
#include "runtime_service_invoker.hpp"
#include "runtime_service_capability_negotiator.hpp"
#include "runtime_service_capability_invoker.hpp"

#include <cstdint>
#include <string_view>

namespace sovereign
{

class RuntimeServiceEndpoint final
{
public:
    static constexpr std::string_view
        RequestTopic{
            "runtime.service.endpoint.request"};

    static constexpr std::string_view
        ResponseTopic{
            "runtime.service.endpoint.response"};

    RuntimeServiceEndpoint(
        RuntimeMessageBus& messaging,
        RuntimeServiceInvoker& invoker,
        RuntimeServiceDispatcher& dispatcher);

    RuntimeServiceEndpoint(
        RuntimeMessageBus& messaging,
        RuntimeServiceInvoker& invoker,
        RuntimeServiceDispatcher& dispatcher,
        RuntimeServiceCapabilityNegotiator&
            negotiator);

    RuntimeServiceEndpoint(
        RuntimeMessageBus& messaging,
        RuntimeServiceInvoker& invoker,
        RuntimeServiceDispatcher& dispatcher,
        RuntimeServiceCapabilityNegotiator&
            negotiator,
        RuntimeServiceCapabilityInvoker&
            capability_invoker);

    RuntimeServiceEndpoint(
        const RuntimeServiceEndpoint&) = delete;

    RuntimeServiceEndpoint&
    operator=(
        const RuntimeServiceEndpoint&) = delete;

    ~RuntimeServiceEndpoint();

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
    std::uint64_t DirectCount() const noexcept;

    [[nodiscard]]
    std::uint64_t RoutedCount() const noexcept;

    [[nodiscard]]
    std::uint64_t CapabilityCount() const noexcept;

    [[nodiscard]]
    std::uint64_t CapabilityInvocationCount()
        const noexcept;

    [[nodiscard]]
    std::uint64_t MalformedCount() const noexcept;

private:
    void HandleRequest(const Event& event);

    RuntimeMessageBus& messaging_;
    RuntimeServiceInvoker& invoker_;
    RuntimeServiceDispatcher& dispatcher_;

    RuntimeServiceCapabilityNegotiator*
        negotiator_{nullptr};

    RuntimeServiceCapabilityInvoker*
        capability_invoker_{nullptr};

    SubscriptionId subscription_id_{
        SubscriptionId::Nil()};

    bool running_{false};

    std::uint64_t request_count_{0U};
    std::uint64_t response_count_{0U};
    std::uint64_t direct_count_{0U};
    std::uint64_t routed_count_{0U};
    std::uint64_t capability_count_{0U};
    std::uint64_t capability_invocation_count_{0U};
    std::uint64_t malformed_count_{0U};
};

} // namespace sovereign
