#include "runtime_message_bus.hpp"
#include "runtime_message_request.hpp"
#include "runtime_message_response.hpp"

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

    const sovereign::TransactionId transaction_id =
        sovereign::TransactionId::Generate();

    const sovereign::DecisionId decision_id =
        sovereign::DecisionId::Generate();

    const sovereign::FacultyId faculty_id =
        sovereign::FacultyId::Generate();

    const sovereign::AgentId agent_id =
        sovereign::AgentId::Generate();

    sovereign::Event captured_response{};
    bool response_captured = false;

    REQUIRE(
        messaging.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.helper.response",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::
                        Confidential,
                .retry_policy = {},
                .handler =
                    [&captured_response,
                     &response_captured](
                        const sovereign::Event& event)
                    {
                        captured_response = event;
                        response_captured = true;
                    }}));

    REQUIRE(
        messaging.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.helper.request",
                .category =
                    sovereign::EventCategory::Runtime,
                .minimum_priority =
                    sovereign::EventPriority::Critical,
                .security_clearance =
                    sovereign::EventSecurity::
                        Confidential,
                .retry_policy = {},
                .handler =
                    [&messaging](
                        const sovereign::Event& event)
                    {
                        const auto response =
                            messaging.Respond(
                                event,
                                "runtime.helper.response",
                                "reply:" + event.payload);

                        REQUIRE(response.Accepted());
                    }}));

    const std::size_t subscriptions_before_request =
        messaging.SubscriptionCount();

    const auto result =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic =
                    "runtime.helper.request",
                .response_topic =
                    "runtime.helper.response",
                .payload = "ping",
                .category =
                    sovereign::EventCategory::Runtime,
                .priority =
                    sovereign::EventPriority::Critical,
                .security =
                    sovereign::EventSecurity::
                        Confidential,
                .transaction_id =
                    transaction_id,
                .decision_id =
                    decision_id,
                .source_faculty =
                    faculty_id,
                .source_agent =
                    agent_id,
                .maximum_process_cycles = 2U});

    REQUIRE(result.Completed());
    REQUIRE(result.response_payload == "reply:ping");
    REQUIRE(response_captured);

    REQUIRE(
        captured_response.metadata.correlation_id ==
        result.correlation_id);

    REQUIRE(
        captured_response.metadata.transaction_id ==
        transaction_id);

    REQUIRE(
        captured_response.metadata.decision_id ==
        decision_id);

    REQUIRE(
        captured_response.metadata.source_faculty ==
        faculty_id);

    REQUIRE(
        captured_response.metadata.source_agent ==
        agent_id);

    REQUIRE(
        captured_response.metadata.category ==
        sovereign::EventCategory::Runtime);

    REQUIRE(
        captured_response.metadata.priority ==
        sovereign::EventPriority::Critical);

    REQUIRE(
        captured_response.metadata.security ==
        sovereign::EventSecurity::Confidential);

    REQUIRE(
        captured_response.metadata.source_runtime ==
        runtime_id);

    REQUIRE(
        captured_response.metadata.source_node ==
        node_id);

    REQUIRE(
        messaging.SubscriptionCount() ==
        subscriptions_before_request);

    /*
     * Verify explicit response overrides while retaining
     * the mandatory request correlation identity.
     */
    sovereign::Event overridden_response{};
    bool override_captured = false;

    REQUIRE(
        messaging.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.override.response",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Restricted,
                .retry_policy = {},
                .handler =
                    [&overridden_response,
                     &override_captured](
                        const sovereign::Event& event)
                    {
                        overridden_response = event;
                        override_captured = true;
                    }}));

    REQUIRE(
        messaging.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.override.request",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Restricted,
                .retry_policy = {},
                .handler =
                    [&messaging](
                        const sovereign::Event& event)
                    {
                        const auto response =
                            messaging.Respond(
                                event,
                                sovereign::RuntimeMessageResponse{
                                    .topic =
                                        "runtime.override.response",
                                    .payload =
                                        "overridden",
                                    .category =
                                        sovereign::EventCategory::
                                            Security,
                                    .priority =
                                        sovereign::EventPriority::
                                            Emergency,
                                    .security =
                                        sovereign::EventSecurity::
                                            Restricted});

                        REQUIRE(response.Accepted());
                    }}));

    const auto override_result =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic =
                    "runtime.override.request",
                .response_topic =
                    "runtime.override.response",
                .payload = "request",
                .security =
                    sovereign::EventSecurity::Restricted,
                .maximum_process_cycles = 2U});

    REQUIRE(override_result.Completed());
    REQUIRE(override_captured);

    REQUIRE(
        overridden_response.metadata.correlation_id ==
        override_result.correlation_id);

    REQUIRE(
        overridden_response.metadata.category ==
        sovereign::EventCategory::Security);

    REQUIRE(
        overridden_response.metadata.priority ==
        sovereign::EventPriority::Emergency);

    REQUIRE(
        overridden_response.metadata.security ==
        sovereign::EventSecurity::Restricted);

    /*
     * Invalid response topics use the existing publication
     * validation path.
     */
    sovereign::Event standalone_request{};
    standalone_request.metadata.correlation_id =
        sovereign::CorrelationId::Generate();

    const auto invalid_response =
        messaging.Respond(
            standalone_request,
            "",
            "invalid");

    REQUIRE(!invalid_response.Accepted());

    REQUIRE(
        invalid_response.status ==
        sovereign::RuntimeMessagePublishStatus::
            EmptyTopic);

    return 0;
}
