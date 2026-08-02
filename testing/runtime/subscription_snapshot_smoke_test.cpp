#include "event_bus.hpp"

#include <cstddef>
#include <string>
#include "test_require.hpp"

using namespace sovereign;

int main()
{
    EventBus bus;

    std::size_t deliveries{0U};

    EventSubscription subscription{
        .topic = "runtime.snapshot",
        .category = EventCategory::Runtime,
        .minimum_priority = EventPriority::Normal,
        .security_clearance =
            EventSecurity::Internal,
        .retry_policy =
            EventRetryPolicy{
                .maximum_attempts = 3U},
        .handler =
            [&deliveries](const Event&)
            {
                ++deliveries;
            }};

    REQUIRE(bus.Subscribe(subscription));
    REQUIRE(bus.SubscriptionCount() == 1U);

    const auto identity =
        bus.SubscriptionIdentity(0U);

    REQUIRE(identity.has_value());
    REQUIRE(!identity->IsNil());

    const auto initial_by_id =
        bus.SubscriptionSnapshotInfo(*identity);

    const auto initial_by_index =
        bus.SubscriptionSnapshotAt(0U);

    REQUIRE(initial_by_id.has_value());
    REQUIRE(initial_by_index.has_value());

    REQUIRE(initial_by_id->index == 0U);
    REQUIRE(initial_by_id->subscription_id == *identity);
    REQUIRE(initial_by_id->handler_available);
    REQUIRE(initial_by_id->Enabled());

    REQUIRE(
        initial_by_id->subscription_id ==
        initial_by_index->subscription_id);

    REQUIRE(
        initial_by_id->configuration ==
        initial_by_index->configuration);

    REQUIRE(
        initial_by_id->lifecycle.revision ==
        initial_by_index->lifecycle.revision);

    REQUIRE(
        initial_by_id->configuration.topic ==
        "runtime.snapshot");

    REQUIRE(
        initial_by_id
            ->configuration
            .category ==
        EventCategory::Runtime);

    REQUIRE(
        initial_by_id
            ->configuration
            .minimum_priority ==
        EventPriority::Normal);

    REQUIRE(
        initial_by_id
            ->configuration
            .security_clearance ==
        EventSecurity::Internal);

    REQUIRE(
        initial_by_id
            ->configuration
            .retry_policy
            .maximum_attempts == 3U);

    const auto created_at =
        initial_by_id->lifecycle.created_at;

    const auto initial_revision =
        initial_by_id->lifecycle.revision;

    REQUIRE(bus.DisableSubscription(*identity));

    const auto disabled =
        bus.SubscriptionSnapshotInfo(*identity);

    REQUIRE(disabled.has_value());
    REQUIRE(!disabled->Enabled());
    REQUIRE(disabled->index == 0U);
    REQUIRE(disabled->subscription_id == *identity);
    REQUIRE(disabled->handler_available);
    REQUIRE(disabled->lifecycle.created_at == created_at);
    REQUIRE(
        disabled->lifecycle.revision ==
        initial_revision + 1U);

    SubscriptionConfiguration updated =
        disabled->configuration;

    updated.topic = "runtime.snapshot.updated";
    updated.minimum_priority =
        EventPriority::High;
    updated.retry_policy.maximum_attempts = 5U;

    REQUIRE(
        bus.ConfigureSubscription(
            *identity,
            updated));

    const auto configured =
        bus.SubscriptionSnapshotAt(0U);

    REQUIRE(configured.has_value());
    REQUIRE(!configured->Enabled());
    REQUIRE(configured->subscription_id == *identity);
    REQUIRE(configured->lifecycle.created_at == created_at);
    REQUIRE(
        configured->lifecycle.revision ==
        initial_revision + 2U);

    REQUIRE(
        configured->configuration.topic ==
        "runtime.snapshot.updated");

    REQUIRE(
        configured
            ->configuration
            .minimum_priority ==
        EventPriority::High);

    REQUIRE(
        configured
            ->configuration
            .retry_policy
            .maximum_attempts == 5U);

    REQUIRE(
        bus.ConfigureSubscription(
            *identity,
            configured->configuration));

    const auto idempotent =
        bus.SubscriptionSnapshotInfo(*identity);

    REQUIRE(idempotent.has_value());
    REQUIRE(
        idempotent->lifecycle.revision ==
        configured->lifecycle.revision);

    REQUIRE(bus.EnableSubscription(*identity));

    const auto enabled =
        bus.SubscriptionSnapshotInfo(*identity);

    REQUIRE(enabled.has_value());
    REQUIRE(enabled->Enabled());
    REQUIRE(enabled->lifecycle.created_at == created_at);
    REQUIRE(
        enabled->lifecycle.revision ==
        initial_revision + 3U);

    REQUIRE(
        !bus.SubscriptionSnapshotInfo(
                SubscriptionId::Nil())
             .has_value());

    REQUIRE(
        !bus.SubscriptionSnapshotAt(1U)
             .has_value());

    REQUIRE(bus.Unsubscribe(*identity));

    REQUIRE(
        !bus.SubscriptionSnapshotInfo(*identity)
             .has_value());

    REQUIRE(
        !bus.SubscriptionSnapshotAt(0U)
             .has_value());

    REQUIRE(deliveries == 0U);

    return 0;
}
