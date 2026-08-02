#include "event_bus.hpp"
#include "identity_types.hpp"
#include "subscription_lifecycle.hpp"

#include <cstddef>
#include <stdexcept>
#include "test_require.hpp"

int main()
{
    sovereign::EventBus bus{8U};

    const auto subscription_id =
        sovereign::SubscriptionId::Generate();

    std::size_t calls = 0U;
    bool reject_delivery = false;

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .subscription_id =
                    subscription_id,
                .topic =
                    "runtime.subscription.lifecycle",
                .retry_policy =
                    sovereign::EventRetryPolicy{
                        .maximum_attempts = 1U},
                .handler =
                    [&calls,
                     &reject_delivery](
                        const sovereign::Event&)
                    {
                        ++calls;

                        if (reject_delivery)
                        {
                            throw std::runtime_error{
                                "lifecycle rejection"};
                        }
                    }}));

    REQUIRE(bus.HasSubscription(subscription_id));
    REQUIRE(bus.SubscriptionEnabled(subscription_id));

    const auto initial =
        bus.SubscriptionLifecycleInfo(
            subscription_id);

    REQUIRE(initial.has_value());
    REQUIRE(initial->subscription_id ==
           subscription_id);
    REQUIRE(initial->Enabled());
    REQUIRE(initial->revision == 0U);
    REQUIRE(initial->created_at ==
           initial->updated_at);

    sovereign::Event event;
    event.topic =
        "runtime.subscription.lifecycle";

    REQUIRE(bus.Publish(event));

    const auto enabled_delivery =
        bus.ProcessWithResult();

    REQUIRE(enabled_delivery.evaluated == 1U);
    REQUIRE(enabled_delivery.matched == 1U);
    REQUIRE(enabled_delivery.delivered == 1U);
    REQUIRE(calls == 1U);

    /*
     * Disable the subscription. Repeating the operation is
     * idempotent and must not increment the lifecycle revision.
     */
    REQUIRE(
        bus.DisableSubscription(
            subscription_id));

    REQUIRE(
        bus.DisableSubscription(
            subscription_id));

    REQUIRE(!bus.SubscriptionEnabled(
        subscription_id));

    const auto disabled =
        bus.SubscriptionLifecycleInfo(
            subscription_id);

    REQUIRE(disabled.has_value());
    REQUIRE(!disabled->Enabled());
    REQUIRE(disabled->revision == 1U);
    REQUIRE(disabled->updated_at >=
           disabled->created_at);

    REQUIRE(bus.Publish(event));

    const auto disabled_delivery =
        bus.ProcessWithResult();

    REQUIRE(disabled_delivery.evaluated == 0U);
    REQUIRE(disabled_delivery.matched == 0U);
    REQUIRE(disabled_delivery.delivered == 0U);
    REQUIRE(calls == 1U);

    /*
     * Re-enable the subscription. Repeating enable is also
     * idempotent.
     */
    REQUIRE(
        bus.EnableSubscription(
            subscription_id));

    REQUIRE(
        bus.EnableSubscription(
            subscription_id));

    REQUIRE(bus.SubscriptionEnabled(
        subscription_id));

    const auto reenabled =
        bus.SubscriptionLifecycleInfo(
            subscription_id);

    REQUIRE(reenabled.has_value());
    REQUIRE(reenabled->Enabled());
    REQUIRE(reenabled->revision == 2U);
    REQUIRE(reenabled->created_at ==
           initial->created_at);
    REQUIRE(reenabled->updated_at >=
           disabled->updated_at);

    REQUIRE(bus.Publish(event));

    const auto resumed_delivery =
        bus.ProcessWithResult();

    REQUIRE(resumed_delivery.evaluated == 1U);
    REQUIRE(resumed_delivery.matched == 1U);
    REQUIRE(resumed_delivery.delivered == 1U);
    REQUIRE(calls == 2U);

    /*
     * Produce a dead letter while enabled.
     */
    reject_delivery = true;

    REQUIRE(bus.Publish(event));

    const auto rejected =
        bus.ProcessWithResult();

    REQUIRE(rejected.exhausted == 1U);
    REQUIRE(bus.DeadLetters().Size() == 1U);
    REQUIRE(calls == 3U);

    /*
     * Disable the target before replay. The stable target still
     * exists, but replay must retain the dead letter rather than
     * invoke a disabled handler.
     */
    REQUIRE(
        bus.DisableSubscription(
            subscription_id));

    const auto disabled_replay =
        bus.ReplayDeadLetter(0U);

    REQUIRE(disabled_replay.found);
    REQUIRE(disabled_replay.target_found);
    REQUIRE(!disabled_replay.delivered);
    REQUIRE(disabled_replay.retained);
    REQUIRE(!disabled_replay.Complete());
    REQUIRE(disabled_replay.subscription_id ==
           subscription_id);
    REQUIRE(calls == 3U);

    REQUIRE(bus.DeadLetters().Size() == 1U);

    const auto& retained =
        bus.DeadLetters().Entries().front();

    REQUIRE(retained.subscription_id ==
           subscription_id);
    REQUIRE(retained.replay_count == 1U);
    REQUIRE(retained.failure_reason ==
           "subscription target disabled");

    /*
     * Re-enable and recover. Replay should now invoke the exact
     * stable subscription and erase the dead letter.
     */
    reject_delivery = false;

    REQUIRE(
        bus.EnableSubscription(
            subscription_id));

    const auto successful_replay =
        bus.ReplayDeadLetter(0U);

    REQUIRE(successful_replay.found);
    REQUIRE(successful_replay.target_found);
    REQUIRE(successful_replay.delivered);
    REQUIRE(!successful_replay.retained);
    REQUIRE(successful_replay.Complete());
    REQUIRE(successful_replay.subscription_id ==
           subscription_id);
    REQUIRE(successful_replay.replay_count == 2U);
    REQUIRE(calls == 4U);
    REQUIRE(bus.DeadLetters().Empty());

    const auto missing_id =
        sovereign::SubscriptionId::Generate();

    REQUIRE(
        !bus.EnableSubscription(
            missing_id));

    REQUIRE(
        !bus.DisableSubscription(
            missing_id));

    REQUIRE(
        !bus.SubscriptionEnabled(
            missing_id));

    REQUIRE(
        !bus.SubscriptionLifecycleInfo(
             missing_id)
             .has_value());

    REQUIRE(
        !bus.EnableSubscription(
            sovereign::SubscriptionId::Nil()));

    REQUIRE(
        !bus.DisableSubscription(
            sovereign::SubscriptionId::Nil()));

    return 0;
}
