#include "event_bus.hpp"

#include <cstddef>
#include "test_require.hpp"

using namespace sovereign;

int main()
{
    EventBus bus;

    for (std::size_t index = 0U;
         index < 5U;
         ++index)
    {
        EventSubscription subscription{
            .topic =
                index % 2U == 0U
                    ? "execution.even"
                    : "execution.odd",
            .category = EventCategory::Runtime,
            .minimum_priority =
                EventPriority::Normal,
            .security_clearance =
                EventSecurity::Internal,
            .handler =
                [](const Event&)
                {
                }};

        REQUIRE(bus.Subscribe(subscription));
    }

    /*
     * A normal first page reports the complete execution
     * shape independently of returned entry snapshots.
     */
    const auto first_page =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 0U,
                .limit = 2U});

    const auto& first =
        first_page.execution_diagnostics;

    REQUIRE(first.requested_offset == 0U);
    REQUIRE(first.requested_limit == 2U);
    REQUIRE(first.matched_entries == 5U);
    REQUIRE(first.returned_entries == 2U);
    REQUIRE(first.remaining_entries == 3U);
    REQUIRE(first.offset_in_range);
    REQUIRE(!first.metadata_only);
    REQUIRE(!first.terminal_page);
    REQUIRE(first_page.has_more);

    /*
     * A final partial page is terminal and reports no
     * remaining entries.
     */
    const auto final_page =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 4U,
                .limit = 3U});

    const auto& final =
        final_page.execution_diagnostics;

    REQUIRE(final.requested_offset == 4U);
    REQUIRE(final.requested_limit == 3U);
    REQUIRE(final.matched_entries == 5U);
    REQUIRE(final.returned_entries == 1U);
    REQUIRE(final.remaining_entries == 0U);
    REQUIRE(final.offset_in_range);
    REQUIRE(!final.metadata_only);
    REQUIRE(final.terminal_page);
    REQUIRE(!final_page.has_more);

    /*
     * Zero-limit requests preserve result metadata without
     * returning subscription entries.
     */
    const auto metadata_only =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 2U,
                .limit = 0U});

    const auto& metadata =
        metadata_only.execution_diagnostics;

    REQUIRE(metadata_only.entries.empty());
    REQUIRE(metadata.requested_offset == 2U);
    REQUIRE(metadata.requested_limit == 0U);
    REQUIRE(metadata.matched_entries == 5U);
    REQUIRE(metadata.returned_entries == 0U);
    REQUIRE(metadata.remaining_entries == 3U);
    REQUIRE(metadata.offset_in_range);
    REQUIRE(metadata.metadata_only);
    REQUIRE(!metadata.terminal_page);

    /*
     * An out-of-range offset produces a terminal page.
     */
    const auto beyond =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 20U,
                .limit = 2U});

    const auto& terminal =
        beyond.execution_diagnostics;

    REQUIRE(beyond.entries.empty());
    REQUIRE(terminal.matched_entries == 5U);
    REQUIRE(terminal.returned_entries == 0U);
    REQUIRE(terminal.remaining_entries == 0U);
    REQUIRE(!terminal.offset_in_range);
    REQUIRE(!terminal.metadata_only);
    REQUIRE(terminal.terminal_page);

    /*
     * Execution diagnostics describe filtered matches,
     * not the complete unfiltered inventory.
     */
    const auto filtered =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query =
                    SubscriptionInventoryQuery{
                        .topic =
                            "execution.even"},
                .sort = {},
                .offset = 1U,
                .limit = 1U});

    const auto& filtered_execution =
        filtered.execution_diagnostics;

    REQUIRE(filtered_execution.matched_entries == 3U);
    REQUIRE(filtered_execution.returned_entries == 1U);
    REQUIRE(filtered_execution.remaining_entries == 1U);
    REQUIRE(filtered_execution.offset_in_range);
    REQUIRE(!filtered_execution.terminal_page);

    /*
     * Diagnostics are detached value metadata.
     */
    auto detached =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 0U,
                .limit = 1U});

    detached.execution_diagnostics.matched_entries =
        999U;

    const auto unchanged =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 0U,
                .limit = 1U});

    REQUIRE(
        unchanged.execution_diagnostics
            .matched_entries == 5U);

    return 0;
}
