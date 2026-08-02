#include "runtime_message_bus.hpp"

#include <optional>
#include <utility>

namespace sovereign
{

RuntimeMessageBus::RuntimeMessageBus(
    EventBus& event_bus)
    :
    RuntimeMessageBus(
        event_bus,
        RuntimeId::Generate(),
        NodeId::Generate())
{
}

RuntimeMessageBus::RuntimeMessageBus(
    EventBus& event_bus,
    RuntimeId runtime_id,
    NodeId node_id)
    :
    event_bus_(event_bus),
    runtime_id_(
        runtime_id.IsNil()
            ? RuntimeId::Generate()
            : runtime_id),
    node_id_(
        node_id.IsNil()
            ? NodeId::Generate()
            : node_id)
{
}

RuntimeMessagePublishResult
RuntimeMessageBus::Publish(
    RuntimeMessage message)
{
    RuntimeMessagePublishResult result{};
    result.topic = message.topic;
    result.pending_before =
        event_bus_.PendingCount();
    result.pending_after =
        result.pending_before;

    if (message.topic.empty())
    {
        result.status =
            RuntimeMessagePublishStatus::EmptyTopic;

        last_publish_result_ = result;
        return result;
    }

    Event event{};

    event.metadata.category =
        message.category;
    event.metadata.priority =
        message.priority;
    event.metadata.security =
        message.security;
    event.metadata.correlation_id =
        message.correlation_id;
    event.metadata.transaction_id =
        message.transaction_id;
    event.metadata.decision_id =
        message.decision_id;
    event.metadata.source_runtime =
        runtime_id_;
    event.metadata.source_node =
        node_id_;
    event.metadata.source_faculty =
        message.source_faculty;
    event.metadata.source_agent =
        message.source_agent;
    event.metadata.sequence =
        next_sequence_;

    event.topic =
        std::move(message.topic);
    event.payload =
        std::move(message.payload);

    result.event_id =
        event.metadata.event_id;
    result.sequence =
        event.metadata.sequence;

    if (!event_bus_.Publish(std::move(event)))
    {
        result.status =
            RuntimeMessagePublishStatus::
                QueueRejected;

        last_publish_result_ = result;
        return result;
    }

    ++next_sequence_;
    ++published_count_;

    result.status =
        RuntimeMessagePublishStatus::Accepted;
    result.pending_after =
        event_bus_.PendingCount();

    last_publish_result_ = result;
    return result;
}

RuntimeMessagePublishResult
RuntimeMessageBus::Publish(
    std::string_view topic,
    std::string payload)
{
    return Publish(
        RuntimeMessage{
            .topic = std::string{topic},
            .payload = std::move(payload)});
}

bool RuntimeMessageBus::Subscribe(
    EventSubscription subscription)
{
    return event_bus_.Subscribe(
        std::move(subscription));
}

bool RuntimeMessageBus::Unsubscribe(
    const SubscriptionId&
        subscription_id) noexcept
{
    return event_bus_.Unsubscribe(
        subscription_id);
}

RuntimeMessageRequestResult
RuntimeMessageBus::Request(
    RuntimeMessageRequest request)
{
    RuntimeMessageRequestResult result{};
    result.request_topic =
        request.request_topic;
    result.response_topic =
        request.response_topic;

    if (request.request_topic.empty())
    {
        result.status =
            RuntimeMessageRequestStatus::
                EmptyRequestTopic;

        return result;
    }

    if (request.response_topic.empty())
    {
        result.status =
            RuntimeMessageRequestStatus::
                EmptyResponseTopic;

        return result;
    }

    const CorrelationId correlation_id =
        CorrelationId::Generate();

    result.correlation_id =
        correlation_id;

    const SubscriptionId response_subscription_id =
        SubscriptionId::Generate();

    std::optional<Event> response{};

    const bool subscribed =
        Subscribe(
            EventSubscription{
                .subscription_id =
                    response_subscription_id,
                .topic =
                    request.response_topic,
                .category =
                    std::nullopt,
                .minimum_priority =
                    std::nullopt,
                .security_clearance =
                    request.security,
                .retry_policy = {},
                .handler =
                    [&response, &correlation_id](
                        const Event& event)
                    {
                        if (event.metadata.correlation_id ==
                            correlation_id)
                        {
                            response = event;
                        }
                    }});

    if (!subscribed)
    {
        result.status =
            RuntimeMessageRequestStatus::
                SubscriptionRejected;

        return result;
    }

    const RuntimeMessagePublishResult publish_result =
        Publish(
            RuntimeMessage{
                .topic =
                    std::move(
                        request.request_topic),
                .payload =
                    std::move(request.payload),
                .category =
                    request.category,
                .priority =
                    request.priority,
                .security =
                    request.security,
                .correlation_id =
                    correlation_id,
                .transaction_id =
                    request.transaction_id,
                .decision_id =
                    request.decision_id,
                .source_faculty =
                    request.source_faculty,
                .source_agent =
                    request.source_agent});

    result.request_event_id =
        publish_result.event_id;
    result.request_sequence =
        publish_result.sequence;

    if (!publish_result.Accepted())
    {
        static_cast<void>(
            Unsubscribe(
                response_subscription_id));

        result.status =
            RuntimeMessageRequestStatus::
                RequestRejected;

        return result;
    }

    const std::size_t maximum_cycles =
        request.maximum_process_cycles == 0U
            ? 1U
            : request.maximum_process_cycles;

    for (std::size_t cycle = 0U;
         cycle < maximum_cycles &&
         !response.has_value();
         ++cycle)
    {
        static_cast<void>(
            ProcessWithResult());

        ++result.process_cycles;
    }

    static_cast<void>(
        Unsubscribe(
            response_subscription_id));

    if (!response.has_value())
    {
        result.status =
            RuntimeMessageRequestStatus::
                NoResponse;

        return result;
    }

    result.status =
        RuntimeMessageRequestStatus::
            Completed;

    result.response_event_id =
        response->metadata.event_id;
    result.response_sequence =
        response->metadata.sequence;
    result.response_payload =
        std::move(response->payload);

    return result;
}

RuntimeMessagePublishResult
RuntimeMessageBus::Respond(
    const Event& request,
    RuntimeMessageResponse response)
{
    return Publish(
        RuntimeMessage{
            .topic =
                std::move(response.topic),
            .payload =
                std::move(response.payload),
            .category =
                response.category.value_or(
                    request.metadata.category),
            .priority =
                response.priority.value_or(
                    request.metadata.priority),
            .security =
                response.security.value_or(
                    request.metadata.security),
            .correlation_id =
                request.metadata.correlation_id,
            .transaction_id =
                response.transaction_id.value_or(
                    request.metadata.transaction_id),
            .decision_id =
                response.decision_id.value_or(
                    request.metadata.decision_id),
            .source_faculty =
                response.source_faculty.value_or(
                    request.metadata.source_faculty),
            .source_agent =
                response.source_agent.value_or(
                    request.metadata.source_agent)});
}

RuntimeMessagePublishResult
RuntimeMessageBus::Respond(
    const Event& request,
    std::string_view response_topic,
    std::string response_payload)
{
    return Respond(
        request,
        RuntimeMessageResponse{
            .topic =
                std::string{response_topic},
            .payload =
                std::move(response_payload)});
}

bool RuntimeMessageBus::Process()
{
    return event_bus_.Process();
}

EventDeliveryResult
RuntimeMessageBus::ProcessWithResult()
{
    return event_bus_.ProcessWithResult();
}

std::size_t
RuntimeMessageBus::PendingCount() const
{
    return event_bus_.PendingCount();
}

std::size_t
RuntimeMessageBus::SubscriptionCount() const noexcept
{
    return event_bus_.SubscriptionCount();
}

std::uint64_t
RuntimeMessageBus::PublishedCount() const noexcept
{
    return published_count_;
}

std::uint64_t
RuntimeMessageBus::NextSequence() const noexcept
{
    return next_sequence_;
}

RuntimeId
RuntimeMessageBus::RuntimeIdentity() const noexcept
{
    return runtime_id_;
}

NodeId
RuntimeMessageBus::NodeIdentity() const noexcept
{
    return node_id_;
}

const RuntimeMessagePublishResult&
RuntimeMessageBus::LastPublishResult() const noexcept
{
    return last_publish_result_;
}

EventBus& RuntimeMessageBus::Events() noexcept
{
    return event_bus_;
}

const EventBus&
RuntimeMessageBus::Events() const noexcept
{
    return event_bus_;
}

} // namespace sovereign
