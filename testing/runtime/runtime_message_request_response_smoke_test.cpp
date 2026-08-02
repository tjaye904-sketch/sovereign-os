#include "runtime_message_bus.hpp"
#include "runtime_message_request.hpp"

#include <cstddef>
#include <optional>
#include <string>
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

    const std::size_t initial_subscriptions =
        messaging.SubscriptionCount();

    const auto empty_request =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic = "",
                .response_topic =
                    "runtime.response",
                .payload = "invalid"});

    REQUIRE(!empty_request.Completed());
    REQUIRE(
        empty_request.status ==
        sovereign::RuntimeMessageRequestStatus::
            EmptyRequestTopic);
    REQUIRE(
        messaging.SubscriptionCount() ==
        initial_subscriptions);

    const auto empty_response =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic =
                    "runtime.request",
                .response_topic = "",
                .payload = "invalid"});

    REQUIRE(!empty_response.Completed());
    REQUIRE(
        empty_response.status ==
        sovereign::RuntimeMessageRequestStatus::
            EmptyResponseTopic);
    REQUIRE(
        messaging.SubscriptionCount() ==
        initial_subscriptions);

    std::size_t request_deliveries = 0U;
    sovereign::CorrelationId observed_correlation =
        sovereign::CorrelationId::Nil();

    REQUIRE(
        messaging.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.request",
                .category =
                    sovereign::EventCategory::Runtime,
                .minimum_priority =
                    sovereign::EventPriority::High,
                .security_clearance =
                    sovereign::EventSecurity::Restricted,
                .retry_policy = {},
                .handler =
                    [&messaging,
                     &request_deliveries,
                     &observed_correlation](
                        const sovereign::Event& event)
                    {
                        ++request_deliveries;

                        observed_correlation =
                            event.metadata.correlation_id;

                        const auto reply =
                            messaging.Publish(
                                sovereign::RuntimeMessage{
                                    .topic =
                                        "runtime.response",
                                    .payload =
                                        "response:" +
                                        event.payload,
                                    .category =
                                        event.metadata.category,
                                    .priority =
                                        event.metadata.priority,
                                    .security =
                                        event.metadata.security,
                                    .correlation_id =
                                        event.metadata
                                            .correlation_id,
                                    .transaction_id =
                                        event.metadata
                                            .transaction_id,
                                    .decision_id =
                                        event.metadata
                                            .decision_id,
                                    .source_faculty =
                                        event.metadata
                                            .source_faculty,
                                    .source_agent =
                                        event.metadata
                                            .source_agent});

                        REQUIRE(reply.Accepted());
                    }}));

    const sovereign::TransactionId transaction_id =
        sovereign::TransactionId::Generate();

    const sovereign::DecisionId decision_id =
        sovereign::DecisionId::Generate();

    const std::size_t subscriptions_before_request =
        messaging.SubscriptionCount();

    const auto completed =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic =
                    "runtime.request",
                .response_topic =
                    "runtime.response",
                .payload = "ping",
                .category =
                    sovereign::EventCategory::Runtime,
                .priority =
                    sovereign::EventPriority::High,
                .security =
                    sovereign::EventSecurity::Restricted,
                .transaction_id =
                    transaction_id,
                .decision_id =
                    decision_id,
                .maximum_process_cycles = 2U});

    REQUIRE(completed.Completed());
    REQUIRE(
        completed.status ==
        sovereign::RuntimeMessageRequestStatus::
            Completed);

    REQUIRE(
        !completed.correlation_id.IsNil());
    REQUIRE(
        completed.correlation_id ==
        observed_correlation);

    REQUIRE(
        !completed.request_event_id.IsNil());
    REQUIRE(
        !completed.response_event_id.IsNil());

    REQUIRE(completed.request_sequence == 1U);
    REQUIRE(completed.response_sequence == 2U);
    REQUIRE(completed.process_cycles == 1U);
    REQUIRE(
        completed.response_payload ==
        "response:ping");

    REQUIRE(request_deliveries == 1U);
    REQUIRE(messaging.PendingCount() == 0U);

    /*
     * The temporary response subscription must be
     * removed after completion.
     */
    REQUIRE(
        messaging.SubscriptionCount() ==
        subscriptions_before_request);

    const auto no_response =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic =
                    "runtime.unhandled",
                .response_topic =
                    "runtime.missing",
                .payload = "no reply",
                .maximum_process_cycles = 3U});

    REQUIRE(!no_response.Completed());
    REQUIRE(
        no_response.status ==
        sovereign::RuntimeMessageRequestStatus::
            NoResponse);
    REQUIRE(
        !no_response.correlation_id.IsNil());
    REQUIRE(
        !no_response.request_event_id.IsNil());

    /*
     * Processing drains the queue during the first
     * cycle. Remaining cycles are still bounded and
     * deterministic.
     */
    REQUIRE(no_response.process_cycles == 3U);
    REQUIRE(messaging.PendingCount() == 0U);

    REQUIRE(
        messaging.SubscriptionCount() ==
        subscriptions_before_request);

    /*
     * Ensure unrelated responses with a different
     * correlation identity are ignored.
     */
    REQUIRE(
        messaging.Subscribe(
            sovereign::EventSubscription{
                .topic =
                    "runtime.correlation.request",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy = {},
                .handler =
                    [&messaging](
                        const sovereign::Event&)
                    {
                        const auto unrelated =
                            messaging.Publish(
                                sovereign::RuntimeMessage{
                                    .topic =
                                        "runtime.correlation.response",
                                    .payload =
                                        "wrong correlation",
                                    .correlation_id =
                                        sovereign::CorrelationId::
                                            Generate()});

                        REQUIRE(unrelated.Accepted());
                    }}));

    const std::size_t subscriptions_before_mismatch =
        messaging.SubscriptionCount();

    const auto mismatch =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic =
                    "runtime.correlation.request",
                .response_topic =
                    "runtime.correlation.response",
                .payload = "correlate",
                .maximum_process_cycles = 2U});

    REQUIRE(!mismatch.Completed());
    REQUIRE(
        mismatch.status ==
        sovereign::RuntimeMessageRequestStatus::
            NoResponse);
    REQUIRE(
        mismatch.response_event_id.IsNil());
    REQUIRE(mismatch.response_payload.empty());

    REQUIRE(
        messaging.SubscriptionCount() ==
        subscriptions_before_mismatch);

    return 0;
}
