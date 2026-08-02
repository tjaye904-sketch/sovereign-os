#include "event.hpp"

#include "test_require.hpp"

int main()
{
    sovereign::Event event;

    REQUIRE(!event.metadata.event_id.IsNil());

    REQUIRE(event.metadata.correlation_id.IsNil());
    REQUIRE(event.metadata.transaction_id.IsNil());
    REQUIRE(event.metadata.decision_id.IsNil());
    REQUIRE(event.metadata.source_runtime.IsNil());
    REQUIRE(event.metadata.source_node.IsNil());
    REQUIRE(event.metadata.source_faculty.IsNil());
    REQUIRE(event.metadata.source_agent.IsNil());

    event.metadata.correlation_id =
        sovereign::CorrelationId::Generate();

    event.metadata.transaction_id =
        sovereign::TransactionId::Generate();

    event.metadata.decision_id =
        sovereign::DecisionId::Generate();

    event.metadata.source_runtime =
        sovereign::RuntimeId::Generate();

    event.metadata.source_node =
        sovereign::NodeId::Generate();

    event.metadata.source_faculty =
        sovereign::FacultyId::Generate();

    event.metadata.source_agent =
        sovereign::AgentId::Generate();

    event.metadata.sequence = 42U;

    REQUIRE(!event.metadata.correlation_id.IsNil());
    REQUIRE(!event.metadata.transaction_id.IsNil());
    REQUIRE(!event.metadata.decision_id.IsNil());
    REQUIRE(!event.metadata.source_runtime.IsNil());
    REQUIRE(!event.metadata.source_node.IsNil());
    REQUIRE(!event.metadata.source_faculty.IsNil());
    REQUIRE(!event.metadata.source_agent.IsNil());
    REQUIRE(event.metadata.sequence == 42U);

    const sovereign::Event copied = event;

    REQUIRE(
        copied.metadata.event_id ==
        event.metadata.event_id);

    REQUIRE(
        copied.metadata.correlation_id ==
        event.metadata.correlation_id);

    REQUIRE(
        copied.metadata.transaction_id ==
        event.metadata.transaction_id);

    REQUIRE(
        copied.metadata.decision_id ==
        event.metadata.decision_id);

    return 0;
}
