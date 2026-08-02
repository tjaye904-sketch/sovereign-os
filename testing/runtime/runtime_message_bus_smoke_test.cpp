#include "runtime.hpp"
#include "runtime_message_bus.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "test_require.hpp"

int main()
{
    sovereign::EventBus event_bus;

    const sovereign::RuntimeId runtime_id =
        sovereign::RuntimeId::Generate();

    const sovereign::NodeId node_id =
        sovereign::NodeId::Generate();

    sovereign::RuntimeMessageBus messaging(
        event_bus,
        runtime_id,
        node_id);

    REQUIRE(
        messaging.RuntimeIdentity() ==
        runtime_id);
    REQUIRE(
        messaging.NodeIdentity() ==
        node_id);

    REQUIRE(messaging.PublishedCount() == 0U);
    REQUIRE(messaging.NextSequence() == 1U);
    REQUIRE(messaging.PendingCount() == 0U);

    const auto empty_result =
        messaging.Publish("", "invalid");

    REQUIRE(!empty_result.Accepted());
    REQUIRE(
        empty_result.status ==
        sovereign::RuntimeMessagePublishStatus::
            EmptyTopic);
    REQUIRE(empty_result.sequence == 0U);
    REQUIRE(empty_result.event_id.IsNil());
    REQUIRE(empty_result.pending_before == 0U);
    REQUIRE(empty_result.pending_after == 0U);
    REQUIRE(messaging.PublishedCount() == 0U);
    REQUIRE(messaging.NextSequence() == 1U);

    std::vector<sovereign::Event> received{};

    REQUIRE(
        messaging.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.message",
                .category =
                    sovereign::EventCategory::Runtime,
                .minimum_priority =
                    sovereign::EventPriority::High,
                .security_clearance =
                    sovereign::EventSecurity::
                        Restricted,
                .retry_policy = {},
                .handler =
                    [&received](
                        const sovereign::Event& event)
                    {
                        received.push_back(event);
                    }}));

    const sovereign::CorrelationId correlation_id =
        sovereign::CorrelationId::Generate();

    const sovereign::TransactionId transaction_id =
        sovereign::TransactionId::Generate();

    const auto first_result =
        messaging.Publish(
            sovereign::RuntimeMessage{
                .topic = "runtime.message",
                .payload = "first",
                .category =
                    sovereign::EventCategory::Runtime,
                .priority =
                    sovereign::EventPriority::High,
                .security =
                    sovereign::EventSecurity::
                        Restricted,
                .correlation_id = correlation_id,
                .transaction_id = transaction_id});

    REQUIRE(first_result.Accepted());
    REQUIRE(!first_result.event_id.IsNil());
    REQUIRE(first_result.topic == "runtime.message");
    REQUIRE(first_result.sequence == 1U);
    REQUIRE(first_result.pending_before == 0U);
    REQUIRE(first_result.pending_after == 1U);

    const auto second_result =
        messaging.Publish(
            "runtime.message",
            "second");

    REQUIRE(second_result.Accepted());
    REQUIRE(second_result.sequence == 2U);
    REQUIRE(second_result.pending_before == 1U);
    REQUIRE(second_result.pending_after == 2U);

    REQUIRE(messaging.PublishedCount() == 2U);
    REQUIRE(messaging.NextSequence() == 3U);
    REQUIRE(messaging.PendingCount() == 2U);

    const auto delivery =
        messaging.ProcessWithResult();

    REQUIRE(delivery.evaluated == 2U);
    REQUIRE(delivery.matched == 1U);
    REQUIRE(delivery.delivered == 1U);
    REQUIRE(delivery.rejected == 0U);

    /*
     * The second message uses default Normal priority,
     * so the High-priority subscription rejects it.
     */
    REQUIRE(received.size() == 1U);

    const sovereign::Event& first =
        received[0U];

    REQUIRE(first.topic == "runtime.message");
    REQUIRE(first.payload == "first");
    REQUIRE(first.metadata.sequence == 1U);
    REQUIRE(
        first.metadata.event_id ==
        first_result.event_id);
    REQUIRE(
        first.metadata.source_runtime ==
        runtime_id);
    REQUIRE(
        first.metadata.source_node ==
        node_id);
    REQUIRE(
        first.metadata.correlation_id ==
        correlation_id);
    REQUIRE(
        first.metadata.transaction_id ==
        transaction_id);
    REQUIRE(
        first.metadata.priority ==
        sovereign::EventPriority::High);
    REQUIRE(
        first.metadata.security ==
        sovereign::EventSecurity::Restricted);

    const auto& last_publish =
        messaging.LastPublishResult();

    REQUIRE(last_publish.Accepted());
    REQUIRE(last_publish.sequence == 2U);

    REQUIRE(messaging.PendingCount() == 0U);

    /*
     * Verify direct Runtime integration while retaining
     * compatibility with the original Events() accessor.
     */
    sovereign::Runtime runtime;

    REQUIRE(
        &runtime.Messaging().Events() ==
        &runtime.Events());

    REQUIRE(
        !runtime.Messaging()
             .RuntimeIdentity()
             .IsNil());

    REQUIRE(
        !runtime.Messaging()
             .NodeIdentity()
             .IsNil());

    std::size_t runtime_deliveries = 0U;

    REQUIRE(
        runtime.Messaging().Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.integrated",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy = {},
                .handler =
                    [&runtime_deliveries](
                        const sovereign::Event&)
                    {
                        ++runtime_deliveries;
                    }}));

    const auto integrated_result =
        runtime.Messaging().Publish(
            "runtime.integrated",
            "online");

    REQUIRE(integrated_result.Accepted());
    REQUIRE(runtime.Messaging().Process());
    REQUIRE(runtime_deliveries == 1U);

    return 0;
}
