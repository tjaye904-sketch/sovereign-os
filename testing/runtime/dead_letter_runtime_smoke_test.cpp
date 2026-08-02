#include "event_bus.hpp"

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include "test_require.hpp"

int main()
{
    sovereign::EventBus bus{2U};

    std::size_t transient_attempts = 0U;

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.transient",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy =
                    sovereign::EventRetryPolicy{
                        .maximum_attempts = 2U},
                .handler =
                    [&transient_attempts](
                        const sovereign::Event&)
                    {
                        ++transient_attempts;

                        if (transient_attempts == 1U)
                        {
                            throw std::runtime_error{
                                "temporary failure"};
                        }
                    }}));

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.failure",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy =
                    sovereign::EventRetryPolicy{
                        .maximum_attempts = 3U},
                .handler =
                    [](const sovereign::Event&)
                    {
                        throw std::runtime_error{
                            "permanent failure"};
                    }}));

    sovereign::Event transient_event;
    transient_event.topic = "runtime.transient";
    transient_event.payload = "recovered";

    REQUIRE(bus.Publish(transient_event));

    const auto transient_result =
        bus.ProcessWithResult();

    REQUIRE(transient_result.delivered == 1U);
    REQUIRE(transient_result.exhausted == 0U);
    REQUIRE(bus.DeadLetters().Empty());

    sovereign::Event first_failure;
    first_failure.topic = "runtime.failure";
    first_failure.payload = "first";

    REQUIRE(bus.Publish(first_failure));

    const auto first_result =
        bus.ProcessWithResult();

    REQUIRE(first_result.attempts == 3U);
    REQUIRE(first_result.retried == 2U);
    REQUIRE(first_result.rejected == 3U);
    REQUIRE(first_result.exhausted == 1U);

    REQUIRE(bus.DeadLetters().Size() == 1U);

    const auto& first_entry =
        bus.DeadLetters().Entries().front();

    REQUIRE(first_entry.event.topic ==
           "runtime.failure");
    REQUIRE(first_entry.event.payload == "first");
    REQUIRE(first_entry.attempts == 3U);
    REQUIRE(first_entry.failure_reason ==
           "permanent failure");

    sovereign::Event second_failure;
    second_failure.topic = "runtime.failure";
    second_failure.payload = "second";

    sovereign::Event third_failure;
    third_failure.topic = "runtime.failure";
    third_failure.payload = "third";

    REQUIRE(bus.Publish(second_failure));
    REQUIRE(bus.Publish(third_failure));

    const auto overflow_result =
        bus.ProcessWithResult();

    REQUIRE(overflow_result.exhausted == 2U);
    REQUIRE(bus.DeadLetters().Size() == 2U);
    REQUIRE(bus.DeadLetters().Capacity() == 2U);
    REQUIRE(bus.DeadLetters().DiscardedCount() == 1U);

    const auto& entries =
        bus.DeadLetters().Entries();

    REQUIRE(entries[0].event.payload == "second");
    REQUIRE(entries[1].event.payload == "third");

    bus.ClearDeadLetters();

    REQUIRE(bus.DeadLetters().Empty());
    REQUIRE(bus.DeadLetters().DiscardedCount() == 1U);

    sovereign::EventBus disabled_bus{0U};

    REQUIRE(
        disabled_bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.disabled",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy = {},
                .handler =
                    [](const sovereign::Event&)
                    {
                        throw 42;
                    }}));

    sovereign::Event disabled_event;
    disabled_event.topic = "runtime.disabled";

    REQUIRE(disabled_bus.Publish(disabled_event));

    const auto disabled_result =
        disabled_bus.ProcessWithResult();

    REQUIRE(disabled_result.exhausted == 1U);
    REQUIRE(disabled_bus.DeadLetters().Empty());
    REQUIRE(
        disabled_bus.DeadLetters().DiscardedCount() ==
        1U);

    return 0;
}
