#include "event_bus.hpp"

#include <cstddef>
#include <iostream>
#include <optional>
#include <utility>
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
        if (page.entries[index].index != expected[index])
        {
            std::cerr << "sort mismatch at position "
                      << index
                      << "\nexpected:";

            for (const std::size_t value : expected)
            {
                std::cerr << ' ' << value;
            }

            std::cerr << "\nactual:  ";

            for (const SubscriptionSnapshot& entry :
                 page.entries)
            {
                std::cerr << ' ' << entry.index;
            }

            std::cerr << '\n';
            REQUIRE(false);
        }
    }
}

SubscriptionInventoryPage QuerySorted(
    const EventBus& bus,
    std::vector<SubscriptionInventorySortTerm> terms,
    const std::size_t offset = 0U,
    const std::size_t limit = 64U)
{
    return bus.QuerySubscriptionInventoryPage(
        SubscriptionInventoryPageRequest{
            .query = {},
            .sort =
                SubscriptionInventorySort{
                    .terms = std::move(terms)},
            .offset = offset,
            .limit = limit});
}

} // namespace

int main()
{
    EventBus bus;

    /*
     * index topic category priority security enabled
     *
     * 0 beta  Runtime  Normal   Internal     true
     * 1 alpha Runtime  High     Public       false
     * 2 beta  Security Low      Restricted   false
     * 3 alpha Security Critical Internal     true
     * 4 beta  Runtime  High     Public       true
     * 5 alpha Runtime  Normal   Restricted   false
     */
    const std::vector<EventSubscription> subscriptions{
        EventSubscription{
            .topic = "beta",
            .category = EventCategory::Runtime,
            .minimum_priority = EventPriority::Normal,
            .security_clearance = EventSecurity::Internal,
            .retry_policy = {},
            .handler = [](const Event&) {}},
        EventSubscription{
            .topic = "alpha",
            .category = EventCategory::Runtime,
            .minimum_priority = EventPriority::High,
            .security_clearance = EventSecurity::Public,
            .retry_policy = {},
            .handler = [](const Event&) {}},
        EventSubscription{
            .topic = "beta",
            .category = EventCategory::Security,
            .minimum_priority = EventPriority::Low,
            .security_clearance = EventSecurity::Restricted,
            .retry_policy = {},
            .handler = [](const Event&) {}},
        EventSubscription{
            .topic = "alpha",
            .category = EventCategory::Security,
            .minimum_priority = EventPriority::Critical,
            .security_clearance = EventSecurity::Internal,
            .retry_policy = {},
            .handler = [](const Event&) {}},
        EventSubscription{
            .topic = "beta",
            .category = EventCategory::Runtime,
            .minimum_priority = EventPriority::High,
            .security_clearance = EventSecurity::Public,
            .retry_policy = {},
            .handler = [](const Event&) {}},
        EventSubscription{
            .topic = "alpha",
            .category = EventCategory::Runtime,
            .minimum_priority = EventPriority::Normal,
            .security_clearance = EventSecurity::Restricted,
            .retry_policy = {},
            .handler = [](const Event&) {}}};

    for (const EventSubscription& subscription :
         subscriptions)
    {
        REQUIRE(bus.Subscribe(subscription));
    }

    const auto identity_1 =
        bus.SubscriptionIdentity(1U);
    const auto identity_2 =
        bus.SubscriptionIdentity(2U);
    const auto identity_5 =
        bus.SubscriptionIdentity(5U);

    REQUIRE(identity_1.has_value());
    REQUIRE(identity_2.has_value());
    REQUIRE(identity_5.has_value());

    REQUIRE(bus.DisableSubscription(*identity_1));
    REQUIRE(bus.DisableSubscription(*identity_2));
    REQUIRE(bus.DisableSubscription(*identity_5));

    /*
     * Empty terms preserve default inventory order.
     */
    AssertIndices(
        QuerySorted(bus, {}),
        {0U, 1U, 2U, 3U, 4U, 5U});

    /*
     * One explicit term behaves like RT-025.
     */
    AssertIndices(
        QuerySorted(
            bus,
            {
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::Topic,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Ascending}
            }),
        {1U, 3U, 5U, 0U, 2U, 4U});

    /*
     * Topic ascending, category descending.
     */
    AssertIndices(
        QuerySorted(
            bus,
            {
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::Topic,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Ascending},
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::Category,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Descending}
            }),
        {3U, 1U, 5U, 2U, 0U, 4U});

    /*
     * Enabled descending, topic ascending,
     * minimum priority descending.
     */
    AssertIndices(
        QuerySorted(
            bus,
            {
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::Enabled,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Descending},
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::Topic,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Ascending},
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::
                            MinimumPriority,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Descending}
            }),
        {3U, 4U, 0U, 1U, 5U, 2U});

    /*
     * Security ascending, topic descending.
     */
    AssertIndices(
        QuerySorted(
            bus,
            {
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::
                            SecurityClearance,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Ascending},
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::Topic,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Descending}
            }),
        {4U, 1U, 0U, 3U, 2U, 5U});

    /*
     * Duplicate equal terms remain deterministic.
     */
    AssertIndices(
        QuerySorted(
            bus,
            {
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::Topic,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Ascending},
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::Topic,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Descending}
            }),
        {1U, 3U, 5U, 0U, 2U, 4U});

    /*
     * An explicit inventory-index term obeys its direction.
     */
    AssertIndices(
        QuerySorted(
            bus,
            {
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::
                            InventoryIndex,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Descending}
            }),
        {5U, 4U, 3U, 2U, 1U, 0U});

    /*
     * Sorting occurs before pagination.
     */
    const auto page =
        QuerySorted(
            bus,
            {
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::Topic,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Ascending},
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::
                            MinimumPriority,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Descending}
            },
            1U,
            3U);

    AssertIndices(page, {1U, 5U, 4U});
    REQUIRE(page.offset == 1U);
    REQUIRE(page.limit == 3U);
    REQUIRE(page.total_matches == 6U);
    REQUIRE(page.has_more);

    /*
     * Filtering precedes multi-field sorting.
     */
    const auto filtered =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query =
                    SubscriptionInventoryQuery{
                        .enabled = false},
                .sort =
                    SubscriptionInventorySort{
                        .terms =
                            {
                                SubscriptionInventorySortTerm{
                                    .field =
                                        SubscriptionInventorySortField::
                                            Topic,
                                    .direction =
                                        SubscriptionInventorySortDirection::
                                            Descending},
                                SubscriptionInventorySortTerm{
                                    .field =
                                        SubscriptionInventorySortField::
                                            MinimumPriority,
                                    .direction =
                                        SubscriptionInventorySortDirection::
                                            Ascending}
                            }},
                .offset = 0U,
                .limit = 64U});

    AssertIndices(filtered, {2U, 5U, 1U});
    REQUIRE(filtered.total_matches == 3U);
    REQUIRE(!filtered.has_more);

    /*
     * Legacy fields are ignored when explicit terms exist.
     */
    const auto explicit_terms_override =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort =
                    SubscriptionInventorySort{
                        .field =
                            SubscriptionInventorySortField::
                                InventoryIndex,
                        .direction =
                            SubscriptionInventorySortDirection::
                                Descending,
                        .terms =
                            {
                                SubscriptionInventorySortTerm{
                                    .field =
                                        SubscriptionInventorySortField::
                                            Topic,
                                    .direction =
                                        SubscriptionInventorySortDirection::
                                            Ascending}
                            }},
                .offset = 0U,
                .limit = 64U});

    AssertIndices(
        explicit_terms_override,
        {1U, 3U, 5U, 0U, 2U, 4U});

    /*
     * Returned snapshots remain detached.
     */
    auto detached =
        QuerySorted(
            bus,
            {
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::Topic,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Ascending}
            });

    detached.entries[0U].configuration.topic =
        "changed";

    const auto unchanged =
        QuerySorted(
            bus,
            {
                SubscriptionInventorySortTerm{
                    .field =
                        SubscriptionInventorySortField::Topic,
                    .direction =
                        SubscriptionInventorySortDirection::
                            Ascending}
            });

    REQUIRE(
        unchanged.entries[0U].configuration.topic ==
        "alpha");

    return 0;
}
