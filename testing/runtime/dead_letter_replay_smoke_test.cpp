#include "event_bus.hpp"
#include "identity_types.hpp"

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include "test_require.hpp"

int main()
{
    sovereign::EventBus bus{8U};

    const auto successful_subscription_id =
        sovereign::SubscriptionId::Generate();

    const auto recovering_subscription_id =
        sovereign::SubscriptionId::Generate();

    std::size_t successful_handler_calls = 0U;
    std::size_t recovering_handler_calls = 0U;
    bool recovery_enabled = false;

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .subscription_id =
                    successful_subscription_id,
                .topic = "runtime.replay",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy = {},
                .handler =
                    [&successful_handler_calls](
                        const sovereign::Event&)
                    {
                        ++successful_handler_calls;
                    }}));

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .subscription_id =
                    recovering_subscription_id,
                .topic = "runtime.replay",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy =
                    sovereign::EventRetryPolicy{
                        .maximum_attempts = 2U},
                .handler =
                    [&recovering_handler_calls,
                     &recovery_enabled](
                        const sovereign::Event&)
                    {
                        ++recovering_handler_calls;

                        if (!recovery_enabled)
                        {
                            throw std::runtime_error{
                                "dependency unavailable"};
                        }
                    }}));

    sovereign::Event event;
    event.topic = "runtime.replay";
    event.payload = "preserved replay payload";

    REQUIRE(bus.Publish(event));

    const auto initial =
        bus.ProcessWithResult();

    REQUIRE(initial.matched == 2U);
    REQUIRE(initial.delivered == 1U);
    REQUIRE(initial.exhausted == 1U);

    REQUIRE(successful_handler_calls == 1U);
    REQUIRE(recovering_handler_calls == 2U);

    REQUIRE(bus.DeadLetters().Size() == 1U);

    const auto& initial_dead_letter =
        bus.DeadLetters().Entries().front();

    REQUIRE(initial_dead_letter.subscription_id ==
           recovering_subscription_id);
    REQUIRE(initial_dead_letter.attempts == 2U);
    REQUIRE(initial_dead_letter.replay_count == 0U);
    REQUIRE(initial_dead_letter.event.payload ==
           "preserved replay payload");

    const auto invalid =
        bus.ReplayDeadLetter(99U);

    REQUIRE(!invalid.found);
    REQUIRE(!invalid.delivered);
    REQUIRE(!invalid.retained);
    REQUIRE(bus.DeadLetters().Size() == 1U);

    const auto failed_replay =
        bus.ReplayDeadLetter(0U);

    REQUIRE(failed_replay.found);
    REQUIRE(failed_replay.target_found);
    REQUIRE(!failed_replay.delivered);
    REQUIRE(failed_replay.retained);
    REQUIRE(failed_replay.subscription_id ==
           recovering_subscription_id);
    REQUIRE(failed_replay.replay_count == 1U);
    REQUIRE(failed_replay.delivery.attempts == 2U);
    REQUIRE(failed_replay.delivery.exhausted == 1U);

    REQUIRE(successful_handler_calls == 1U);
    REQUIRE(recovering_handler_calls == 4U);

    REQUIRE(bus.DeadLetters().Size() == 1U);

    const auto& retained =
        bus.DeadLetters().Entries().front();

    REQUIRE(retained.subscription_id ==
           recovering_subscription_id);
    REQUIRE(retained.attempts == 4U);
    REQUIRE(retained.replay_count == 1U);
    REQUIRE(retained.failure_reason ==
           "dependency unavailable");
    REQUIRE(retained.event.payload ==
           "preserved replay payload");

    recovery_enabled = true;

    const auto successful_replay =
        bus.ReplayDeadLetter(0U);

    REQUIRE(successful_replay.found);
    REQUIRE(successful_replay.target_found);
    REQUIRE(successful_replay.delivered);
    REQUIRE(!successful_replay.retained);
    REQUIRE(successful_replay.Complete());
    REQUIRE(successful_replay.subscription_id ==
           recovering_subscription_id);
    REQUIRE(successful_replay.replay_count == 2U);
    REQUIRE(successful_replay.delivery.attempts == 1U);
    REQUIRE(successful_replay.delivery.delivered == 1U);
    REQUIRE(successful_replay.delivery.exhausted == 0U);

    REQUIRE(successful_handler_calls == 1U);
    REQUIRE(recovering_handler_calls == 5U);
    REQUIRE(bus.DeadLetters().Empty());

    return 0;
}
