#include "event_bus.hpp"

#include <cstddef>
#include <string>
#include <vector>
#include "test_require.hpp"

using namespace sovereign;

int main()
{
    EventBus bus;

    const auto empty =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{});

    REQUIRE(empty.entries.empty());
    REQUIRE(empty.offset == 0U);
    REQUIRE(
        empty.limit ==
        DefaultSubscriptionInventoryPageLimit);
    REQUIRE(empty.total_matches == 0U);
    REQUIRE(!empty.has_more);

    std::vector<SubscriptionId> identities;
    identities.reserve(7U);

    for (std::size_t index = 0U;
         index < 7U;
         ++index)
    {
        const bool even = index % 2U == 0U;

        EventSubscription subscription{
            .topic =
                even
                    ? "page.even"
                    : "page.odd",
            .category =
                even
                    ? EventCategory::Runtime
                    : EventCategory::Security,
            .minimum_priority =
                even
                    ? EventPriority::Normal
                    : EventPriority::High,
            .security_clearance =
                even
                    ? EventSecurity::Internal
                    : EventSecurity::Restricted,
            .retry_policy =
                EventRetryPolicy{
                    .maximum_attempts =
                        index + 1U},
            .handler =
                [](const Event&)
                {
                }};

        REQUIRE(bus.Subscribe(subscription));

        const auto identity =
            bus.SubscriptionIdentity(index);

        REQUIRE(identity.has_value());
        identities.push_back(*identity);
    }

    REQUIRE(bus.SubscriptionCount() == 7U);

    /*
     * First bounded page.
     */
    const auto first_page =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 0U,
                .limit = 3U});

    REQUIRE(first_page.entries.size() == 3U);
    REQUIRE(first_page.offset == 0U);
    REQUIRE(first_page.limit == 3U);
    REQUIRE(first_page.total_matches == 7U);
    REQUIRE(first_page.has_more);

    for (std::size_t index = 0U;
         index < first_page.entries.size();
         ++index)
    {
        REQUIRE(
            first_page.entries[index].index ==
            index);
        REQUIRE(
            first_page.entries[index].subscription_id ==
            identities[index]);
    }

    /*
     * Middle page uses positions within the filtered
     * result set while snapshots retain inventory indices.
     */
    const auto middle_page =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 3U,
                .limit = 3U});

    REQUIRE(middle_page.entries.size() == 3U);
    REQUIRE(middle_page.offset == 3U);
    REQUIRE(middle_page.limit == 3U);
    REQUIRE(middle_page.total_matches == 7U);
    REQUIRE(middle_page.has_more);

    for (std::size_t index = 0U;
         index < middle_page.entries.size();
         ++index)
    {
        const std::size_t inventory_index =
            index + 3U;

        REQUIRE(
            middle_page.entries[index].index ==
            inventory_index);
        REQUIRE(
            middle_page.entries[index].subscription_id ==
            identities[inventory_index]);
    }

    /*
     * Final partial page.
     */
    const auto final_page =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 6U,
                .limit = 3U});

    REQUIRE(final_page.entries.size() == 1U);
    REQUIRE(final_page.offset == 6U);
    REQUIRE(final_page.limit == 3U);
    REQUIRE(final_page.total_matches == 7U);
    REQUIRE(!final_page.has_more);
    REQUIRE(final_page.entries[0U].index == 6U);
    REQUIRE(
        final_page.entries[0U].subscription_id ==
        identities[6U]);

    /*
     * A limit larger than the remaining result count
     * returns all remaining entries.
     */
    const auto oversized_limit =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 5U,
                .limit = 100U});

    REQUIRE(oversized_limit.entries.size() == 2U);
    REQUIRE(oversized_limit.total_matches == 7U);
    REQUIRE(!oversized_limit.has_more);
    REQUIRE(oversized_limit.entries[0U].index == 5U);
    REQUIRE(oversized_limit.entries[1U].index == 6U);

    /*
     * A zero limit remains a valid metadata-only page.
     */
    const auto zero_limit =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 2U,
                .limit = 0U});

    REQUIRE(zero_limit.entries.empty());
    REQUIRE(zero_limit.offset == 2U);
    REQUIRE(zero_limit.limit == 0U);
    REQUIRE(zero_limit.total_matches == 7U);
    REQUIRE(!zero_limit.has_more);

    /*
     * An offset equal to or beyond total matches returns
     * an empty terminal page.
     */
    const auto terminal_offset =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 7U,
                .limit = 3U});

    REQUIRE(terminal_offset.entries.empty());
    REQUIRE(terminal_offset.total_matches == 7U);
    REQUIRE(!terminal_offset.has_more);

    const auto beyond_offset =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 20U,
                .limit = 3U});

    REQUIRE(beyond_offset.entries.empty());
    REQUIRE(beyond_offset.total_matches == 7U);
    REQUIRE(!beyond_offset.has_more);

    /*
     * Pagination is applied after filtering.
     * Matching inventory indices are 0, 2, 4, and 6.
     */
    const SubscriptionInventoryQuery even_query{
        .topic = "page.even",
        .category = EventCategory::Runtime,
        .minimum_priority =
            EventPriority::Normal,
        .security_clearance =
            EventSecurity::Internal,
        .enabled = true,
        .handler_available = true};

    const auto filtered_first =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = even_query,
                .sort = {},
                .offset = 0U,
                .limit = 2U});

    REQUIRE(filtered_first.entries.size() == 2U);
    REQUIRE(filtered_first.total_matches == 4U);
    REQUIRE(filtered_first.has_more);
    REQUIRE(filtered_first.entries[0U].index == 0U);
    REQUIRE(filtered_first.entries[1U].index == 2U);
    REQUIRE(
        filtered_first.entries[0U].subscription_id ==
        identities[0U]);
    REQUIRE(
        filtered_first.entries[1U].subscription_id ==
        identities[2U]);

    const auto filtered_second =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = even_query,
                .sort = {},
                .offset = 2U,
                .limit = 2U});

    REQUIRE(filtered_second.entries.size() == 2U);
    REQUIRE(filtered_second.total_matches == 4U);
    REQUIRE(!filtered_second.has_more);
    REQUIRE(filtered_second.entries[0U].index == 4U);
    REQUIRE(filtered_second.entries[1U].index == 6U);
    REQUIRE(
        filtered_second.entries[0U].subscription_id ==
        identities[4U]);
    REQUIRE(
        filtered_second.entries[1U].subscription_id ==
        identities[6U]);

    const auto no_matches =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query =
                    SubscriptionInventoryQuery{
                        .topic = "page.missing"},
                .sort = {},
                .offset = 0U,
                .limit = 2U});

    REQUIRE(no_matches.entries.empty());
    REQUIRE(no_matches.total_matches == 0U);
    REQUIRE(!no_matches.has_more);

    /*
     * Returned entries are detached value snapshots.
     */
    auto detached =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 0U,
                .limit = 1U});

    REQUIRE(detached.entries.size() == 1U);

    detached.entries[0U].configuration.topic =
        "mutated.copy";
    detached.entries.clear();

    const auto unchanged =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 0U,
                .limit = 1U});

    REQUIRE(unchanged.entries.size() == 1U);
    REQUIRE(
        unchanged.entries[0U].configuration.topic ==
        "page.even");

    /*
     * Inventory compaction is reflected by future pages,
     * while stable subscription identities remain intact.
     */
    REQUIRE(bus.Unsubscribe(identities[1U]));

    const auto compacted =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 0U,
                .limit = 10U});

    REQUIRE(compacted.entries.size() == 6U);
    REQUIRE(compacted.total_matches == 6U);
    REQUIRE(!compacted.has_more);

    REQUIRE(compacted.entries[0U].index == 0U);
    REQUIRE(compacted.entries[1U].index == 1U);
    REQUIRE(
        compacted.entries[0U].subscription_id ==
        identities[0U]);
    REQUIRE(
        compacted.entries[1U].subscription_id ==
        identities[2U]);

    return 0;
}
