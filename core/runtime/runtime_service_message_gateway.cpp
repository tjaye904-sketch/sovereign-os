#include "runtime_service_message_gateway.hpp"

#include "runtime_service_message_protocol.hpp"

#include <optional>
#include <string>
#include <utility>

namespace sovereign
{

RuntimeServiceMessageGateway::
RuntimeServiceMessageGateway(
    RuntimeMessageBus& messaging,
    RuntimeServiceInvoker& invoker)
    :
    messaging_(messaging),
    invoker_(invoker)
{
}

RuntimeServiceMessageGateway::
~RuntimeServiceMessageGateway()
{
    static_cast<void>(Stop());
}

bool RuntimeServiceMessageGateway::Start()
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

bool RuntimeServiceMessageGateway::Stop() noexcept
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
RuntimeServiceMessageGateway::Running() const noexcept
{
    return running_;
}

std::uint64_t
RuntimeServiceMessageGateway::
RequestCount() const noexcept
{
    return request_count_;
}

std::uint64_t
RuntimeServiceMessageGateway::
ResponseCount() const noexcept
{
    return response_count_;
}

std::uint64_t
RuntimeServiceMessageGateway::
MalformedCount() const noexcept
{
    return malformed_count_;
}

void RuntimeServiceMessageGateway::HandleRequest(
    const Event& event)
{
    ++request_count_;

    RuntimeServiceInvocationResult result{};

    const auto decoded =
        RuntimeServiceMessageProtocol::
            DecodeRequest(event.payload);

    if (!decoded.has_value())
    {
        ++malformed_count_;

        result.status =
            RuntimeServiceInvocationStatus::
                EmptyServiceName;

        result.diagnostic =
            "Malformed runtime service invocation "
            "message.";
    }
    else
    {
        result =
            invoker_.Invoke(
                RuntimeServiceInvocationRequest{
                    .service_name =
                        decoded->service_name,
                    .operation =
                        decoded->operation,
                    .payload =
                        decoded->payload});
    }

    const auto response =
        messaging_.Respond(
            event,
            ResponseTopic,
            RuntimeServiceMessageProtocol::
                EncodeResult(result));

    if (response.Accepted())
    {
        ++response_count_;
    }
}

} // namespace sovereign
