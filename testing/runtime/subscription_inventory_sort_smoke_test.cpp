#include "event_bus.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "test_require.hpp"

using namespace sovereign;

namespace
{

void AssertIndices(
    const SubscriptionInventoryPage& page,
    const std::vector<std::size_t>& expected)
{
    REQUIRE(page.entries.size() == expected.size());

    for (std::size_t index = 0U;
         index < expected.size();
         ++index)
    {
        REQUIRE(
            page.entries[index].index ==
            expected[index]);
    }
}

SubscriptionInventoryPage QuerySorted(
    const EventBus& bus,
    const SubscriptionInventorySortField field,
    const SubscriptionInventorySortDirection direction,
    const std::size_t offset = 0U,
    const std::size_t limit = 64U)
{
    return bus.QuerySubscriptionInventoryPage(
        SubscriptionInventoryPageRequest{
            .query = {},
            .sort =
                SubscriptionInventorySort{
                    .field = field,
                    .direction = direction},
            .offset = offset,
            .limit = limit});
}

} // namespace

int main()
{
    EventBus bus;

    /*
     * Inventory:
     *
     * index  topic    category  priority    security      enabled
     * 0      beta     Runtime   Normal      Internal      true
     * 1      alpha    null      null        Public        false
     * 2      alpha    Security  High        Restricted    true
     * 3      gamma    Runtime   Low         Internal      false
     * 4      beta     null      Critical    Confidential  true
     */
    const std::vector<EventSubscription> subscriptions{
        EventSubscription{
            .topic = "beta",
            .category = EventCategory::Runtime,
            .minimum_priority = EventPriority::Normal,
            .security_clearance = EventSecurity::Internal,
            .retry_policy = {},
            .handler =
                [](const Event&)
                {
                }},
        EventSubscription{
            .topic = "alpha",
            .category = std::nullopt,
            .minimum_priority = std::nullopt,
            .security_clearance = EventSecurity::Public,
            .retry_policy = {},
            .handler =
                [](const Event&)
                {
                }},
        EventSubscription{
            .topic = "alpha",
            .category = EventCategory::Security,
            .minimum_priority = EventPriority::High,
            .security_clearance = EventSecurity::Restricted,
            .retry_policy = {},
            .handler =
                [](const Event&)
                {
                }},
        EventSubscription{
            .topic = "gamma",
            .category = EventCategory::Runtime,
            .minimum_priority = EventPriority::Low,
            .security_clearance = EventSecurity::Internal,
            .retry_policy = {},
            .handler =
                [](const Event&)
                {
                }},
        EventSubscription{
            .topic = "beta",
            .category = std::nullopt,
            .minimum_priority = EventPriority::Critical,
            .security_clearance =
                EventSecurity::Confidential,
            .retry_policy = {},
            .handler =
                [](const Event&)
                {
                }}};

    for (const EventSubscription& subscription :
         subscriptions)
    {
        REQUIRE(bus.Subscribe(subscription));
    }

    REQUIRE(bus.SubscriptionCount() == 5U);

    /*
     * Establish mixed lifecycle state.
     */
    const auto identity_1 =
        bus.SubscriptionIdentity(1U);
    const auto identity_3 =
        bus.SubscriptionIdentity(3U);

    REQUIRE(identity_1.has_value());
    REQUIRE(identity_3.has_value());

    REQUIRE(bus.DisableSubscription(*identity_1));
    REQUIRE(bus.DisableSubscription(*identity_3));

    /*
     * Default ordering remains inventory order.
     */
    const auto default_page =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{});

    AssertIndices(default_page, {0U, 1U, 2U, 3U, 4U});

    /*
     * Inventory index.
     */
    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::InventoryIndex,
            SubscriptionInventorySortDirection::Ascending),
        {0U, 1U, 2U, 3U, 4U});

    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::InventoryIndex,
            SubscriptionInventorySortDirection::Descending),
        {4U, 3U, 2U, 1U, 0U});

    /*
     * Topic sorting with inventory-index tie-breaking.
     */
    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::Topic,
            SubscriptionInventorySortDirection::Ascending),
        {1U, 2U, 0U, 4U, 3U});

    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::Topic,
            SubscriptionInventorySortDirection::Descending),
        {3U, 0U, 4U, 1U, 2U});

    /*
     * Optional category values sort before concrete values
     * ascending and after concrete values descending.
     */
    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::Category,
            SubscriptionInventorySortDirection::Ascending),
        {1U, 4U, 0U, 3U, 2U});

    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::Category,
            SubscriptionInventorySortDirection::Descending),
        {2U, 0U, 3U, 1U, 4U});

    /*
     * Optional minimum-priority ordering.
     */
    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::MinimumPriority,
            SubscriptionInventorySortDirection::Ascending),
        {1U, 3U, 0U, 2U, 4U});

    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::MinimumPriority,
            SubscriptionInventorySortDirection::Descending),
        {4U, 2U, 0U, 3U, 1U});

    /*
     * Security enum ordering.
     */
    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::SecurityClearance,
            SubscriptionInventorySortDirection::Ascending),
        {1U, 0U, 3U, 2U, 4U});

    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::SecurityClearance,
            SubscriptionInventorySortDirection::Descending),
        {4U, 2U, 0U, 3U, 1U});

    /*
     * Boolean fields sort false before true ascending,
     * with inventory-index tie-breaking.
     */
    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::Enabled,
            SubscriptionInventorySortDirection::Ascending),
        {1U, 3U, 0U, 2U, 4U});

    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::Enabled,
            SubscriptionInventorySortDirection::Descending),
        {0U, 2U, 4U, 1U, 3U});

    /*
     * Accepted subscriptions always have handlers under the
     * current subscription contract. Equal primary values use
     * inventory index ascending in both directions.
     */
    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::HandlerAvailable,
            SubscriptionInventorySortDirection::Ascending),
        {0U, 1U, 2U, 3U, 4U});

    AssertIndices(
        QuerySorted(
            bus,
            SubscriptionInventorySortField::HandlerAvailable,
            SubscriptionInventorySortDirection::Descending),
        {0U, 1U, 2U, 3U, 4U});

    /*
     * Sorting precedes pagination.
     */
    const auto topic_page =
        QuerySorted(
            bus,
            SubscriptionInventorySortField::Topic,
            SubscriptionInventorySortDirection::Ascending,
            1U,
            2U);

    AssertIndices(topic_page, {2U, 0U});
    REQUIRE(topic_page.offset == 1U);
    REQUIRE(topic_page.limit == 2U);
    REQUIRE(topic_page.total_matches == 5U);
    REQUIRE(topic_page.has_more);

    /*
     * Filtering precedes sorting and pagination.
     * Enabled entries are 0, 2, and 4.
     */
    const auto filtered =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query =
                    SubscriptionInventoryQuery{
                        .enabled = true},
                .sort =
                    SubscriptionInventorySort{
                        .field =
                            SubscriptionInventorySortField::
                                Topic,
                        .direction =
                            SubscriptionInventorySortDirection::
                                Descending},
                .offset = 0U,
                .limit = 2U});

    AssertIndices(filtered, {0U, 4U});
    REQUIRE(filtered.total_matches == 3U);
    REQUIRE(filtered.has_more);

    /*
     * Returned snapshots remain detached values.
     */
    auto detached =
        QuerySorted(
            bus,
            SubscriptionInventorySortField::Topic,
            SubscriptionInventorySortDirection::Ascending);

    detached.entries[0U].configuration.topic =
        "changed";

    const auto unchanged =
        QuerySorted(
            bus,
            SubscriptionInventorySortField::Topic,
            SubscriptionInventorySortDirection::Ascending);

    REQUIRE(
        unchanged.entries[0U].configuration.topic ==
        "alpha");

    return 0;
}
