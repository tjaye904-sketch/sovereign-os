#include "event_bus.hpp"
#include "identity_types.hpp"
#include "subscription_configuration.hpp"

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
                    "runtime.subscription.original",
                .category =
                    sovereign::EventCategory::Runtime,
                .minimum_priority =
                    sovereign::EventPriority::Normal,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
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
                                "configuration rejection"};
                        }
                    }}));

    REQUIRE(bus.SubscriptionCount() == 1U);
    REQUIRE(bus.HasSubscription(subscription_id));
    REQUIRE(bus.SubscriptionEnabled(subscription_id));

    const auto initial_lifecycle =
        bus.SubscriptionLifecycleInfo(
            subscription_id);

    const auto initial_configuration =
        bus.SubscriptionConfigurationInfo(
            subscription_id);

    REQUIRE(initial_lifecycle.has_value());
    REQUIRE(initial_configuration.has_value());

    REQUIRE(initial_lifecycle->revision == 0U);
    REQUIRE(initial_configuration->topic ==
           "runtime.subscription.original");
    REQUIRE(initial_configuration->category ==
           sovereign::EventCategory::Runtime);
    REQUIRE(initial_configuration->minimum_priority ==
           sovereign::EventPriority::Normal);
    REQUIRE(initial_configuration->security_clearance ==
           sovereign::EventSecurity::Internal);
    REQUIRE(
        initial_configuration
            ->retry_policy
            .maximum_attempts == 1U);

    sovereign::Event original_event;
    original_event.topic =
        "runtime.subscription.original";
    original_event.metadata.category =
        sovereign::EventCategory::Runtime;
    original_event.metadata.priority =
        sovereign::EventPriority::Normal;
    original_event.metadata.security =
        sovereign::EventSecurity::Internal;

    REQUIRE(bus.Publish(original_event));

    const auto original_delivery =
        bus.ProcessWithResult();

    REQUIRE(original_delivery.delivered == 1U);
    REQUIRE(calls == 1U);

    const sovereign::SubscriptionConfiguration updated{
        .topic =
            "runtime.subscription.updated",
        .category =
            sovereign::EventCategory::Security,
        .minimum_priority =
            sovereign::EventPriority::High,
        .security_clearance =
            sovereign::EventSecurity::Restricted,
        .retry_policy =
            sovereign::EventRetryPolicy{
                .maximum_attempts = 2U}};

    REQUIRE(
        bus.ConfigureSubscription(
            subscription_id,
            updated));

    REQUIRE(bus.SubscriptionCount() == 1U);
    REQUIRE(bus.HasSubscription(subscription_id));
    REQUIRE(bus.SubscriptionEnabled(subscription_id));

    const auto updated_lifecycle =
        bus.SubscriptionLifecycleInfo(
            subscription_id);

    const auto updated_configuration =
        bus.SubscriptionConfigurationInfo(
            subscription_id);

    REQUIRE(updated_lifecycle.has_value());
    REQUIRE(updated_configuration.has_value());

    REQUIRE(updated_lifecycle->subscription_id ==
           subscription_id);
    REQUIRE(updated_lifecycle->revision == 1U);
    REQUIRE(updated_lifecycle->created_at ==
           initial_lifecycle->created_at);
    REQUIRE(updated_lifecycle->updated_at >=
           initial_lifecycle->updated_at);

    REQUIRE(*updated_configuration == updated);

    /*
     * The old routing policy must no longer match.
     */
    REQUIRE(bus.Publish(original_event));

    const auto stale_delivery =
        bus.ProcessWithResult();

    REQUIRE(stale_delivery.evaluated == 1U);
    REQUIRE(stale_delivery.matched == 0U);
    REQUIRE(stale_delivery.delivered == 0U);
    REQUIRE(calls == 1U);

    sovereign::Event updated_event;
    updated_event.topic =
        "runtime.subscription.updated";
    updated_event.metadata.category =
        sovereign::EventCategory::Security;
    updated_event.metadata.priority =
        sovereign::EventPriority::High;
    updated_event.metadata.security =
        sovereign::EventSecurity::Restricted;

    REQUIRE(bus.Publish(updated_event));

    const auto updated_delivery =
        bus.ProcessWithResult();

    REQUIRE(updated_delivery.evaluated == 1U);
    REQUIRE(updated_delivery.matched == 1U);
    REQUIRE(updated_delivery.delivered == 1U);
    REQUIRE(calls == 2U);

    /*
     * Reapplying identical configuration is accepted but does
     * not increment the lifecycle revision.
     */
    REQUIRE(
        bus.ConfigureSubscription(
            subscription_id,
            updated));

    const auto idempotent_lifecycle =
        bus.SubscriptionLifecycleInfo(
            subscription_id);

    REQUIRE(idempotent_lifecycle.has_value());
    REQUIRE(idempotent_lifecycle->revision == 1U);
    REQUIRE(idempotent_lifecycle->updated_at ==
           updated_lifecycle->updated_at);

    /*
     * Configuration changes must preserve disabled state.
     */
    REQUIRE(
        bus.DisableSubscription(
            subscription_id));

    const sovereign::SubscriptionConfiguration disabled_update{
        .topic =
            "runtime.subscription.disabled",
        .category =
            std::nullopt,
        .minimum_priority =
            std::nullopt,
        .security_clearance =
            sovereign::EventSecurity::Confidential,
        .retry_policy =
            sovereign::EventRetryPolicy{
                .maximum_attempts = 0U}};

    REQUIRE(
        bus.ConfigureSubscription(
            subscription_id,
            disabled_update));

    REQUIRE(!bus.SubscriptionEnabled(subscription_id));
    REQUIRE(bus.SubscriptionCount() == 1U);

    const auto disabled_lifecycle =
        bus.SubscriptionLifecycleInfo(
            subscription_id);

    REQUIRE(disabled_lifecycle.has_value());
    REQUIRE(disabled_lifecycle->revision == 3U);
    REQUIRE(disabled_lifecycle->created_at ==
           initial_lifecycle->created_at);

    sovereign::Event disabled_event;
    disabled_event.topic =
        "runtime.subscription.disabled";
    disabled_event.metadata.security =
        sovereign::EventSecurity::Confidential;

    REQUIRE(bus.Publish(disabled_event));

    const auto disabled_delivery =
        bus.ProcessWithResult();

    REQUIRE(disabled_delivery.evaluated == 0U);
    REQUIRE(disabled_delivery.delivered == 0U);
    REQUIRE(calls == 2U);

    /*
     * Retry policy value zero remains stored but is normalized
     * to one delivery attempt by EventRetryPolicy.
     */
    REQUIRE(
        bus.EnableSubscription(
            subscription_id));

    reject_delivery = true;

    REQUIRE(bus.Publish(disabled_event));

    const auto rejected_delivery =
        bus.ProcessWithResult();

    REQUIRE(rejected_delivery.attempts == 1U);
    REQUIRE(rejected_delivery.rejected == 1U);
    REQUIRE(rejected_delivery.exhausted == 1U);
    REQUIRE(bus.DeadLetters().Size() == 1U);
    REQUIRE(calls == 3U);

    /*
     * Reconfiguration preserves the stable dead-letter target.
     */
    reject_delivery = false;

    const sovereign::SubscriptionConfiguration replay_update{
        .topic =
            "runtime.subscription.replay",
        .category =
            std::nullopt,
        .minimum_priority =
            std::nullopt,
        .security_clearance =
            sovereign::EventSecurity::Confidential,
        .retry_policy =
            sovereign::EventRetryPolicy{
                .maximum_attempts = 1U}};

    REQUIRE(
        bus.ConfigureSubscription(
            subscription_id,
            replay_update));

    const auto blocked_replay =
        bus.ReplayDeadLetter(0U);

    REQUIRE(blocked_replay.found);
    REQUIRE(blocked_replay.target_found);
    REQUIRE(blocked_replay.subscription_id ==
           subscription_id);
    REQUIRE(!blocked_replay.delivered);
    REQUIRE(!blocked_replay.Complete());
    REQUIRE(bus.DeadLetters().Size() == 1U);
    REQUIRE(calls == 3U);

    /*
     * Restoring a configuration that matches the stored event
     * permits replay through the same stable subscription.
     */
    REQUIRE(
        bus.ConfigureSubscription(
            subscription_id,
            disabled_update));

    const auto replay =
        bus.ReplayDeadLetter(0U);

    REQUIRE(replay.found);
    REQUIRE(replay.target_found);
    REQUIRE(replay.subscription_id ==
           subscription_id);
    REQUIRE(replay.delivered);
    REQUIRE(replay.Complete());
    REQUIRE(bus.DeadLetters().Empty());
    REQUIRE(calls == 4U);

    const auto missing_id =
        sovereign::SubscriptionId::Generate();

    REQUIRE(
        !bus.ConfigureSubscription(
            missing_id,
            updated));

    REQUIRE(
        !bus.ConfigureSubscription(
            sovereign::SubscriptionId::Nil(),
            updated));

    REQUIRE(
        !bus.SubscriptionConfigurationInfo(
             missing_id)
             .has_value());

    REQUIRE(
        !bus.SubscriptionConfigurationInfo(
             sovereign::SubscriptionId::Nil())
             .has_value());

    return 0;
}
