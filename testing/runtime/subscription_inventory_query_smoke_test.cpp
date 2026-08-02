#include "event_bus.hpp"

#include <cstddef>
#include "test_require.hpp"

using namespace sovereign;

int main()
{
    EventBus bus;

    const SubscriptionInventoryQuery empty_query{};

    REQUIRE(
        bus.QuerySubscriptionInventory(
               empty_query)
            .empty());

    EventSubscription first{
        .topic = "query.alpha",
        .category = EventCategory::Runtime,
        .minimum_priority = EventPriority::Normal,
        .security_clearance =
            EventSecurity::Internal,
        .retry_policy =
            EventRetryPolicy{
                .maximum_attempts = 2U},
        .handler =
            [](const Event&)
            {
            }};

    EventSubscription second{
        .topic = "query.beta",
        .category = EventCategory::Security,
        .minimum_priority = EventPriority::High,
        .security_clearance =
            EventSecurity::Restricted,
        .retry_policy =
            EventRetryPolicy{
                .maximum_attempts = 4U},
        .handler =
            [](const Event&)
            {
            }};

    EventSubscription third{
        .topic = "query.alpha",
        .category = EventCategory::Runtime,
        .minimum_priority = EventPriority::Critical,
        .security_clearance =
            EventSecurity::Confidential,
        .retry_policy =
            EventRetryPolicy{
                .maximum_attempts = 6U},
        .handler =
            [](const Event&)
            {
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

    /*
     * An empty query returns the complete inventory while
     * preserving router order and original indices.
     */
    const auto all =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{});

    REQUIRE(all.size() == 3U);
    REQUIRE(all[0U].index == 0U);
    REQUIRE(all[1U].index == 1U);
    REQUIRE(all[2U].index == 2U);

    REQUIRE(all[0U].subscription_id == *first_id);
    REQUIRE(all[1U].subscription_id == *second_id);
    REQUIRE(all[2U].subscription_id == *third_id);

    const auto topic_matches =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{
                .topic = "query.alpha"});

    REQUIRE(topic_matches.size() == 2U);
    REQUIRE(topic_matches[0U].index == 0U);
    REQUIRE(topic_matches[1U].index == 2U);
    REQUIRE(
        topic_matches[0U].subscription_id ==
        *first_id);
    REQUIRE(
        topic_matches[1U].subscription_id ==
        *third_id);

    const auto category_matches =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{
                .category =
                    EventCategory::Security});

    REQUIRE(category_matches.size() == 1U);
    REQUIRE(
        category_matches[0U].subscription_id ==
        *second_id);

    const auto priority_matches =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{
                .minimum_priority =
                    EventPriority::Critical});

    REQUIRE(priority_matches.size() == 1U);
    REQUIRE(
        priority_matches[0U].subscription_id ==
        *third_id);

    const auto security_matches =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{
                .security_clearance =
                    EventSecurity::Restricted});

    REQUIRE(security_matches.size() == 1U);
    REQUIRE(
        security_matches[0U].subscription_id ==
        *second_id);

    const auto handler_matches =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{
                .handler_available = true});

    REQUIRE(handler_matches.size() == 3U);
    REQUIRE(
        handler_matches[0U].subscription_id ==
        *first_id);
    REQUIRE(
        handler_matches[1U].subscription_id ==
        *second_id);
    REQUIRE(
        handler_matches[2U].subscription_id ==
        *third_id);

    const auto unavailable_handlers =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{
                .handler_available = false});

    REQUIRE(unavailable_handlers.empty());

    REQUIRE(bus.DisableSubscription(*second_id));

    const auto disabled_matches =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{
                .enabled = false});

    REQUIRE(disabled_matches.size() == 1U);
    REQUIRE(
        disabled_matches[0U].subscription_id ==
        *second_id);
    REQUIRE(!disabled_matches[0U].Enabled());

    const auto enabled_matches =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{
                .enabled = true});

    REQUIRE(enabled_matches.size() == 2U);
    REQUIRE(
        enabled_matches[0U].subscription_id ==
        *first_id);
    REQUIRE(
        enabled_matches[1U].subscription_id ==
        *third_id);

    /*
     * Multiple populated fields use logical AND.
     */
    const auto combined_matches =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{
                .topic = "query.alpha",
                .category =
                    EventCategory::Runtime,
                .minimum_priority =
                    EventPriority::Critical,
                .security_clearance =
                    EventSecurity::Confidential,
                .enabled = true,
                .handler_available = true});

    REQUIRE(combined_matches.size() == 1U);
    REQUIRE(combined_matches[0U].index == 2U);
    REQUIRE(
        combined_matches[0U].subscription_id ==
        *third_id);

    const auto no_matches =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{
                .topic = "query.missing"});

    REQUIRE(no_matches.empty());

    /*
     * Query results are detached value snapshots.
     */
    auto detached =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{
                .topic = "query.alpha"});

    detached[0U].configuration.topic =
        "mutated.copy";
    detached.clear();

    const auto unchanged =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{
                .topic = "query.alpha"});

    REQUIRE(unchanged.size() == 2U);
    REQUIRE(
        unchanged[0U].configuration.topic ==
        "query.alpha");

    /*
     * Unsubscription compacts inventory indices while stable
     * subscription identities remain unchanged.
     */
    REQUIRE(bus.Unsubscribe(*first_id));

    const auto compacted =
        bus.QuerySubscriptionInventory(
            SubscriptionInventoryQuery{});

    REQUIRE(compacted.size() == 2U);
    REQUIRE(compacted[0U].index == 0U);
    REQUIRE(compacted[1U].index == 1U);
    REQUIRE(
        compacted[0U].subscription_id ==
        *second_id);
    REQUIRE(
        compacted[1U].subscription_id ==
        *third_id);

    REQUIRE(bus.Unsubscribe(*second_id));
    REQUIRE(bus.Unsubscribe(*third_id));

    REQUIRE(
        bus.QuerySubscriptionInventory(
               SubscriptionInventoryQuery{})
            .empty());

    return 0;
}
