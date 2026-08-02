#include "runtime_service_endpoint.hpp"

#include "runtime_service_endpoint_protocol.hpp"

#include <optional>
#include <string>
#include <variant>

namespace sovereign
{

RuntimeServiceEndpoint::RuntimeServiceEndpoint(
    RuntimeMessageBus& messaging,
    RuntimeServiceInvoker& invoker,
    RuntimeServiceDispatcher& dispatcher)
    :
    messaging_(messaging),
    invoker_(invoker),
    dispatcher_(dispatcher)
{
}

RuntimeServiceEndpoint::RuntimeServiceEndpoint(
    RuntimeMessageBus& messaging,
    RuntimeServiceInvoker& invoker,
    RuntimeServiceDispatcher& dispatcher,
    RuntimeServiceCapabilityNegotiator& negotiator)
    :
    messaging_(messaging),
    invoker_(invoker),
    dispatcher_(dispatcher),
    negotiator_(&negotiator)
{
}

RuntimeServiceEndpoint::RuntimeServiceEndpoint(
    RuntimeMessageBus& messaging,
    RuntimeServiceInvoker& invoker,
    RuntimeServiceDispatcher& dispatcher,
    RuntimeServiceCapabilityNegotiator& negotiator,
    RuntimeServiceCapabilityInvoker&
        capability_invoker)
    :
    messaging_(messaging),
    invoker_(invoker),
    dispatcher_(dispatcher),
    negotiator_(&negotiator),
    capability_invoker_(&capability_invoker)
{
}

RuntimeServiceEndpoint::~RuntimeServiceEndpoint()
{
    static_cast<void>(Stop());
}

bool RuntimeServiceEndpoint::Start()
{
    if (running_)
    {
        return false;
    }

    subscription_id_ =
        SubscriptionId::Generate();

    const bool subscribed =
        messaging_.Subscribe(
            EventSubscription{
                .subscription_id =
                    subscription_id_,
                .topic =
                    std::string{RequestTopic},
                .category =
                    EventCategory::Runtime,
                .minimum_priority =
                    std::nullopt,
                .security_clearance =
                    EventSecurity::Restricted,
                .retry_policy = {},
                .handler =
                    [this](const Event& event)
                    {
                        HandleRequest(event);
                    }});

    if (!subscribed)
    {
        subscription_id_ =
            SubscriptionId::Nil();

        return false;
    }

    running_ = true;
    return true;
}

bool RuntimeServiceEndpoint::Stop() noexcept
{
    if (!running_)
    {
        return false;
    }

    const bool removed =
        messaging_.Unsubscribe(
            subscription_id_);

    if (!removed)
    {
        return false;
    }

    running_ = false;

    subscription_id_ =
        SubscriptionId::Nil();

    return true;
}

bool RuntimeServiceEndpoint::Running() const noexcept
{
    return running_;
}

std::uint64_t
RuntimeServiceEndpoint::RequestCount() const noexcept
{
    return request_count_;
}

std::uint64_t
RuntimeServiceEndpoint::ResponseCount() const noexcept
{
    return response_count_;
}

std::uint64_t
RuntimeServiceEndpoint::DirectCount() const noexcept
{
    return direct_count_;
}

std::uint64_t
RuntimeServiceEndpoint::RoutedCount() const noexcept
{
    return routed_count_;
}

std::uint64_t
RuntimeServiceEndpoint::CapabilityCount() const noexcept
{
    return capability_count_;
}

std::uint64_t
RuntimeServiceEndpoint::CapabilityInvocationCount()
    const noexcept
{
    return capability_invocation_count_;
}

std::uint64_t
RuntimeServiceEndpoint::MalformedCount() const noexcept
{
    return malformed_count_;
}

void RuntimeServiceEndpoint::HandleRequest(
    const Event& event)
{
    ++request_count_;

    RuntimeServiceEndpointResult endpoint_result{};

    const auto decoded =
        RuntimeServiceEndpointProtocol::
            DecodeRequest(event.payload);

    if (!decoded.has_value())
    {
        ++malformed_count_;

        RuntimeServiceInvocationResult malformed{};
        malformed.status =
            RuntimeServiceInvocationStatus::
                EmptyServiceName;

        malformed.diagnostic =
            "Malformed unified runtime service "
            "endpoint message.";

        endpoint_result.target =
            RuntimeServiceEndpointTarget::Direct;

        endpoint_result.result =
            std::move(malformed);
    }
    else if (
        decoded->target ==
        RuntimeServiceEndpointTarget::Direct)
    {
        ++direct_count_;

        endpoint_result.target =
            RuntimeServiceEndpointTarget::Direct;

        endpoint_result.result =
            invoker_.Invoke(
                RuntimeServiceInvocationRequest{
                    .service_name =
                        decoded->target_name,
                    .operation =
                        decoded->operation,
                    .payload =
                        decoded->payload});
    }
    else if (
        decoded->target ==
        RuntimeServiceEndpointTarget::Routed)
    {
        ++routed_count_;

        endpoint_result.target =
            RuntimeServiceEndpointTarget::Routed;

        endpoint_result.result =
            dispatcher_.Dispatch(
                RuntimeServiceDispatchRequest{
                    .route_name =
                        decoded->target_name,
                    .operation =
                        decoded->operation,
                    .payload =
                        decoded->payload});
    }
    else if (
        decoded->target ==
        RuntimeServiceEndpointTarget::Capability)
    {
        ++capability_count_;

        endpoint_result.target =
            RuntimeServiceEndpointTarget::
                Capability;

        if (negotiator_ == nullptr)
        {
            RuntimeServiceCapabilityResult
                unavailable{};

            unavailable.status =
                RuntimeServiceCapabilityStatus::
                    NegotiatorUnavailable;

            unavailable.capability =
                decoded->target_name;

            unavailable.diagnostic =
                "The unified runtime service "
                "endpoint has no capability "
                "negotiator.";

            endpoint_result.result =
                std::move(unavailable);
        }
        else
        {
            endpoint_result.result =
                negotiator_->Negotiate(
                    RuntimeServiceCapabilityRequest{
                        .capability =
                            decoded->target_name,
                        .preferred_service_name =
                            decoded->operation,
                        .require_running =
                            decoded->payload != "0"});
        }
    }
    else
    {
        ++capability_invocation_count_;

        endpoint_result.target =
            RuntimeServiceEndpointTarget::
                CapabilityInvocation;

        if (capability_invoker_ == nullptr)
        {
            RuntimeServiceCapabilityInvocationResult
                unavailable{};

            unavailable.status =
                RuntimeServiceCapabilityInvocationStatus::
                    CapabilityInvokerUnavailable;

            unavailable.negotiation.capability =
                decoded->target_name;

            unavailable.diagnostic =
                "The unified runtime service "
                "endpoint has no capability "
                "invoker.";

            endpoint_result.result =
                std::move(unavailable);
        }
        else
        {
            endpoint_result.result =
                capability_invoker_->Invoke(
                    RuntimeServiceCapabilityInvocationRequest{
                        .capability =
                            decoded->target_name,
                        .preferred_service_name = {},
                        .operation =
                            decoded->operation,
                        .payload =
                            decoded->payload});
        }
    }

    const auto response =
        messaging_.Respond(
            event,
            ResponseTopic,
            RuntimeServiceEndpointProtocol::
                EncodeResult(endpoint_result));

    if (response.Accepted())
    {
        ++response_count_;
    }
}

} // namespace sovereign
