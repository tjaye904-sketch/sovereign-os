#include "event_bus.hpp"

#include <cstddef>
#include "test_require.hpp"

using namespace sovereign;

int main()
{
    EventBus bus;

    REQUIRE(bus.SubscriptionInventory().empty());

    std::size_t deliveries{0U};

    EventSubscription first{
        .topic = "inventory.first",
        .category = EventCategory::Runtime,
        .minimum_priority = EventPriority::Normal,
        .security_clearance =
            EventSecurity::Internal,
        .retry_policy =
            EventRetryPolicy{
                .maximum_attempts = 2U},
        .handler =
            [&deliveries](const Event&)
            {
                ++deliveries;
            }};

    EventSubscription second{
        .topic = "inventory.second",
        .category = EventCategory::Runtime,
        .minimum_priority = EventPriority::High,
        .security_clearance =
            EventSecurity::Restricted,
        .retry_policy =
            EventRetryPolicy{
                .maximum_attempts = 4U},
        .handler =
            [&deliveries](const Event&)
            {
                ++deliveries;
            }};

    EventSubscription third{
        .topic = "inventory.third",
        .category = EventCategory::Security,
        .minimum_priority = EventPriority::Critical,
        .security_clearance =
            EventSecurity::Restricted,
        .retry_policy =
            EventRetryPolicy{
                .maximum_attempts = 6U},
        .handler =
            [&deliveries](const Event&)
            {
                ++deliveries;
            }};

    REQUIRE(bus.Subscribe(first));
    REQUIRE(bus.Subscribe(second));
    REQUIRE(bus.Subscribe(third));
    REQUIRE(bus.SubscriptionCount() == 3U);

    const auto first_id =
        bus.SubscriptionIdentity(0U);
    const auto second_id =
        bus.SubscriptionIdentity(1U);
    const auto third_id =
        bus.SubscriptionIdentity(2U);

    REQUIRE(first_id.has_value());
    REQUIRE(second_id.has_value());
    REQUIRE(third_id.has_value());

    auto inventory =
        bus.SubscriptionInventory();

    REQUIRE(inventory.size() == 3U);

    for (std::size_t index = 0U;
         index < inventory.size();
         ++index)
    {
        REQUIRE(inventory[index].index == index);
        REQUIRE(inventory[index].handler_available);
        REQUIRE(inventory[index].Enabled());
    }

    REQUIRE(
        inventory[0U].subscription_id ==
        *first_id);
    REQUIRE(
        inventory[1U].subscription_id ==
        *second_id);
    REQUIRE(
        inventory[2U].subscription_id ==
        *third_id);

    REQUIRE(
        inventory[0U].configuration.topic ==
        "inventory.first");
    REQUIRE(
        inventory[1U].configuration.topic ==
        "inventory.second");
    REQUIRE(
        inventory[2U].configuration.topic ==
        "inventory.third");

    const auto first_created_at =
        inventory[0U].lifecycle.created_at;
    const auto second_created_at =
        inventory[1U].lifecycle.created_at;
    const auto third_created_at =
        inventory[2U].lifecycle.created_at;

    REQUIRE(bus.DisableSubscription(*second_id));

    SubscriptionConfiguration updated =
        inventory[2U].configuration;

    updated.topic = "inventory.third.updated";
    updated.retry_policy.maximum_attempts = 8U;

    REQUIRE(
        bus.ConfigureSubscription(
            *third_id,
            updated));

    const auto changed =
        bus.SubscriptionInventory();

    REQUIRE(changed.size() == 3U);

    REQUIRE(changed[0U].Enabled());
    REQUIRE(!changed[1U].Enabled());
    REQUIRE(changed[2U].Enabled());

    REQUIRE(
        changed[0U].lifecycle.created_at ==
        first_created_at);
    REQUIRE(
        changed[1U].lifecycle.created_at ==
        second_created_at);
    REQUIRE(
        changed[2U].lifecycle.created_at ==
        third_created_at);

    REQUIRE(
        changed[1U].lifecycle.revision ==
        inventory[1U].lifecycle.revision + 1U);

    REQUIRE(
        changed[2U].lifecycle.revision ==
        inventory[2U].lifecycle.revision + 1U);

    REQUIRE(
        changed[2U].configuration.topic ==
        "inventory.third.updated");

    REQUIRE(
        changed[2U]
            .configuration
            .retry_policy
            .maximum_attempts == 8U);

    /*
     * Inventory is a detached value snapshot. Mutating the
     * returned vector must not affect router state.
     */
    inventory[0U].configuration.topic =
        "mutated.copy";
    inventory[0U].handler_available = false;
    inventory.clear();

    const auto detached =
        bus.SubscriptionInventory();

    REQUIRE(detached.size() == 3U);
    REQUIRE(
        detached[0U].configuration.topic ==
        "inventory.first");
    REQUIRE(detached[0U].handler_available);

    REQUIRE(bus.Unsubscribe(*first_id));

    const auto compacted =
        bus.SubscriptionInventory();

    REQUIRE(compacted.size() == 2U);

    REQUIRE(compacted[0U].index == 0U);
    REQUIRE(compacted[1U].index == 1U);

    REQUIRE(
        compacted[0U].subscription_id ==
        *second_id);
    REQUIRE(
        compacted[1U].subscription_id ==
        *third_id);

    REQUIRE(!compacted[0U].Enabled());
    REQUIRE(compacted[1U].Enabled());

    REQUIRE(bus.Unsubscribe(*second_id));
    REQUIRE(bus.Unsubscribe(*third_id));

    REQUIRE(bus.SubscriptionInventory().empty());
    REQUIRE(bus.SubscriptionCount() == 0U);
    REQUIRE(deliveries == 0U);

    return 0;
}
