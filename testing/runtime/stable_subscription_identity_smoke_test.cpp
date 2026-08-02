#include "event_bus.hpp"
#include "identity_types.hpp"

#include <cstddef>
#include <optional>
#include <stdexcept>
#include "test_require.hpp"

int main()
{
    sovereign::EventBus bus{8U};

    const auto first_id =
        sovereign::SubscriptionId::Generate();

    const auto failing_id =
        sovereign::SubscriptionId::Generate();

    const auto third_id =
        sovereign::SubscriptionId::Generate();

    std::size_t first_calls = 0U;
    std::size_t failing_calls = 0U;
    std::size_t third_calls = 0U;

    bool recovery_enabled = false;

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .subscription_id = first_id,
                .topic = "runtime.identity",
                .handler =
                    [&first_calls](
                        const sovereign::Event&)
                    {
                        ++first_calls;
                    }}));

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .subscription_id = failing_id,
                .topic = "runtime.identity",
                .retry_policy =
                    sovereign::EventRetryPolicy{
                        .maximum_attempts = 2U},
                .handler =
                    [&failing_calls,
                     &recovery_enabled](
                        const sovereign::Event&)
                    {
                        ++failing_calls;

                        if (!recovery_enabled)
                        {
                            throw std::runtime_error{
                                "target unavailable"};
                        }
                    }}));

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .subscription_id = third_id,
                .topic = "runtime.identity",
                .handler =
                    [&third_calls](
                        const sovereign::Event&)
                    {
                        ++third_calls;
                    }}));

    REQUIRE(bus.SubscriptionCount() == 3U);
    REQUIRE(bus.HasSubscription(first_id));
    REQUIRE(bus.HasSubscription(failing_id));
    REQUIRE(bus.HasSubscription(third_id));

    REQUIRE(
        bus.SubscriptionIdentity(0U).value() ==
        first_id);

    REQUIRE(
        bus.SubscriptionIdentity(1U).value() ==
        failing_id);

    REQUIRE(
        bus.SubscriptionIdentity(2U).value() ==
        third_id);

    REQUIRE(
        !bus.SubscriptionIdentity(99U)
             .has_value());

    REQUIRE(
        !bus.Subscribe(
            sovereign::EventSubscription{
                .subscription_id = failing_id,
                .topic = "runtime.duplicate",
                .handler =
                    [](const sovereign::Event&)
                    {
                    }}));

    REQUIRE(bus.SubscriptionCount() == 3U);

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.generated",
                .handler =
                    [](const sovereign::Event&)
                    {
                    }}));

    REQUIRE(bus.SubscriptionCount() == 4U);

    const auto generated_id =
        bus.SubscriptionIdentity(3U);

    REQUIRE(generated_id.has_value());
    REQUIRE(!generated_id->IsNil());
    REQUIRE(bus.HasSubscription(*generated_id));

    sovereign::Event event;
    event.topic = "runtime.identity";

    REQUIRE(bus.Publish(event));

    const auto initial =
        bus.ProcessWithResult();

    REQUIRE(initial.matched == 3U);
    REQUIRE(initial.delivered == 2U);
    REQUIRE(initial.exhausted == 1U);

    REQUIRE(first_calls == 1U);
    REQUIRE(failing_calls == 2U);
    REQUIRE(third_calls == 1U);

    REQUIRE(bus.DeadLetters().Size() == 1U);

    const auto& dead_letter =
        bus.DeadLetters().Entries().front();

    REQUIRE(dead_letter.subscription_id ==
           failing_id);

    /*
     * Remove the subscription that preceded the failed target.
     * This shifts the target from vector index 1 to index 0.
     * Replay must still resolve the target by SubscriptionId.
     */
    REQUIRE(bus.Unsubscribe(first_id));
    REQUIRE(!bus.HasSubscription(first_id));
    REQUIRE(bus.SubscriptionCount() == 3U);

    REQUIRE(
        bus.SubscriptionIdentity(0U).value() ==
        failing_id);

    recovery_enabled = true;

    const auto replay =
        bus.ReplayDeadLetter(0U);

    REQUIRE(replay.found);
    REQUIRE(replay.target_found);
    REQUIRE(replay.delivered);
    REQUIRE(!replay.retained);
    REQUIRE(replay.Complete());
    REQUIRE(replay.subscription_id == failing_id);

    REQUIRE(first_calls == 1U);
    REQUIRE(failing_calls == 3U);
    REQUIRE(third_calls == 1U);
    REQUIRE(bus.DeadLetters().Empty());

    /*
     * Create another dead letter, then remove its exact target.
     * Replay must retain the record and report target absence.
     */
    recovery_enabled = false;

    REQUIRE(bus.Publish(event));

    const auto second_delivery =
        bus.ProcessWithResult();

    REQUIRE(second_delivery.exhausted == 1U);
    REQUIRE(bus.DeadLetters().Size() == 1U);

    REQUIRE(bus.Unsubscribe(failing_id));
    REQUIRE(!bus.HasSubscription(failing_id));

    const auto missing_target =
        bus.ReplayDeadLetter(0U);

    REQUIRE(missing_target.found);
    REQUIRE(!missing_target.target_found);
    REQUIRE(!missing_target.delivered);
    REQUIRE(missing_target.retained);
    REQUIRE(!missing_target.Complete());
    REQUIRE(missing_target.subscription_id ==
           failing_id);

    REQUIRE(bus.DeadLetters().Size() == 1U);

    const auto& retained =
        bus.DeadLetters().Entries().front();

    REQUIRE(retained.subscription_id ==
           failing_id);

    REQUIRE(retained.replay_count == 1U);

    REQUIRE(retained.failure_reason ==
           "subscription target unavailable");

    REQUIRE(!bus.Unsubscribe(failing_id));
    REQUIRE(
        !bus.Unsubscribe(
            sovereign::SubscriptionId::Nil()));

    return 0;
}
