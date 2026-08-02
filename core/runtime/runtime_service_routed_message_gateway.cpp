#include "runtime_service_routed_message_gateway.hpp"

#include "runtime_service_routed_message_protocol.hpp"

#include <optional>
#include <string>

namespace sovereign
{

RuntimeServiceRoutedMessageGateway::
RuntimeServiceRoutedMessageGateway(
    RuntimeMessageBus& messaging,
    RuntimeServiceDispatcher& dispatcher)
    :
    messaging_(messaging),
    dispatcher_(dispatcher)
{
}

RuntimeServiceRoutedMessageGateway::
~RuntimeServiceRoutedMessageGateway()
{
    static_cast<void>(Stop());
}

bool RuntimeServiceRoutedMessageGateway::Start()
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

bool
RuntimeServiceRoutedMessageGateway::Stop() noexcept
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

bool
RuntimeServiceRoutedMessageGateway::
Running() const noexcept
{
    return running_;
}

std::uint64_t
RuntimeServiceRoutedMessageGateway::
RequestCount() const noexcept
{
    return request_count_;
}

std::uint64_t
RuntimeServiceRoutedMessageGateway::
ResponseCount() const noexcept
{
    return response_count_;
}

std::uint64_t
RuntimeServiceRoutedMessageGateway::
MalformedCount() const noexcept
{
    return malformed_count_;
}

void
RuntimeServiceRoutedMessageGateway::HandleRequest(
    const Event& event)
{
    ++request_count_;

    RuntimeServiceDispatchResult result{};

    const auto decoded =
        RuntimeServiceRoutedMessageProtocol::
            DecodeRequest(event.payload);

    if (!decoded.has_value())
    {
        ++malformed_count_;

        result.status =
            RuntimeServiceDispatchStatus::
                EmptyRouteName;

        result.diagnostic =
            "Malformed routed runtime service "
            "dispatch message.";
    }
    else
    {
        result =
            dispatcher_.Dispatch(
                RuntimeServiceDispatchRequest{
                    .route_name =
                        decoded->route_name,
                    .operation =
                        decoded->operation,
                    .payload =
                        decoded->payload});
    }

    const auto response =
        messaging_.Respond(
            event,
            ResponseTopic,
            RuntimeServiceRoutedMessageProtocol::
                EncodeResult(result));

    if (response.Accepted())
    {
        ++response_count_;
    }
}

} // namespace sovereign
