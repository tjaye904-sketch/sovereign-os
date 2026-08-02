#pragma once

#include "event_bus.hpp"
#include "event_category.hpp"
#include "event_priority.hpp"
#include "event_security.hpp"
#include "identity_types.hpp"
#include "runtime_message_request.hpp"
#include "runtime_message_response.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace sovereign
{

enum class RuntimeMessagePublishStatus : std::uint8_t
{
    NoAttempt,
    Accepted,
    EmptyTopic,
    QueueRejected
};

struct RuntimeMessage final
{
    std::string topic{};
    std::string payload{};

    EventCategory category{
        EventCategory::Runtime};

    EventPriority priority{
        EventPriority::Normal};

    EventSecurity security{
        EventSecurity::Internal};

    CorrelationId correlation_id{
        CorrelationId::Nil()};

    TransactionId transaction_id{
        TransactionId::Nil()};

    DecisionId decision_id{
        DecisionId::Nil()};

    FacultyId source_faculty{
        FacultyId::Nil()};

    AgentId source_agent{
        AgentId::Nil()};
};

struct RuntimeMessagePublishResult final
{
    RuntimeMessagePublishStatus status{
        RuntimeMessagePublishStatus::NoAttempt};

    EventId event_id{EventId::Nil()};

    std::string topic{};

    std::uint64_t sequence{0U};

    std::size_t pending_before{0U};
    std::size_t pending_after{0U};

    [[nodiscard]]
    bool Accepted() const noexcept
    {
        return status ==
               RuntimeMessagePublishStatus::Accepted;
    }
};

class RuntimeMessageBus final
{
public:
    explicit RuntimeMessageBus(
        EventBus& event_bus);

    RuntimeMessageBus(
        EventBus& event_bus,
        RuntimeId runtime_id,
        NodeId node_id);

    [[nodiscard]]
    RuntimeMessagePublishResult Publish(
        RuntimeMessage message);

    [[nodiscard]]
    RuntimeMessagePublishResult Publish(
        std::string_view topic,
        std::string payload);

    bool Subscribe(
        EventSubscription subscription);

    bool Unsubscribe(
        const SubscriptionId&
            subscription_id) noexcept;

    [[nodiscard]]
    RuntimeMessageRequestResult Request(
        RuntimeMessageRequest request);

    [[nodiscard]]
    RuntimeMessagePublishResult Respond(
        const Event& request,
        RuntimeMessageResponse response);

    [[nodiscard]]
    RuntimeMessagePublishResult Respond(
        const Event& request,
        std::string_view response_topic,
        std::string response_payload);

    bool Process();

    [[nodiscard]]
    EventDeliveryResult ProcessWithResult();

    [[nodiscard]]
    std::size_t PendingCount() const;

    [[nodiscard]]
    std::size_t SubscriptionCount() const noexcept;

    [[nodiscard]]
    std::uint64_t PublishedCount() const noexcept;

    [[nodiscard]]
    std::uint64_t NextSequence() const noexcept;

    [[nodiscard]]
    RuntimeId RuntimeIdentity() const noexcept;

    [[nodiscard]]
    NodeId NodeIdentity() const noexcept;

    [[nodiscard]]
    const RuntimeMessagePublishResult&
    LastPublishResult() const noexcept;

    [[nodiscard]]
    EventBus& Events() noexcept;

    [[nodiscard]]
    const EventBus& Events() const noexcept;

private:
    EventBus& event_bus_;

    RuntimeId runtime_id_;
    NodeId node_id_;

    std::uint64_t next_sequence_{1U};
    std::uint64_t published_count_{0U};

    RuntimeMessagePublishResult
        last_publish_result_{};
};

} // namespace sovereign
