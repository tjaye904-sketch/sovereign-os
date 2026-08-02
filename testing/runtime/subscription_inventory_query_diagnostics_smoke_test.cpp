#include "event_bus.hpp"

#include <cstddef>
#include "test_require.hpp"

using namespace sovereign;

int main()
{
    EventBus bus;

    EventSubscription first{
        .topic = "diagnostics.alpha",
        .category = EventCategory::Runtime,
        .minimum_priority = EventPriority::Normal,
        .security_clearance = EventSecurity::Internal,
        .handler =
            [](const Event&)
            {
            }};

    EventSubscription second{
        .topic = "diagnostics.beta",
        .category = EventCategory::Security,
        .minimum_priority = EventPriority::High,
        .security_clearance =
            EventSecurity::Restricted,
        .handler =
            [](const Event&)
            {
            }};

    EventSubscription third{
        .topic = "diagnostics.alpha",
        .category = EventCategory::Runtime,
        .minimum_priority = EventPriority::Critical,
        .security_clearance =
            EventSecurity::Confidential,
        .handler =
            [](const Event&)
            {
            }};

    REQUIRE(bus.Subscribe(first));
    REQUIRE(bus.Subscribe(second));
    REQUIRE(bus.Subscribe(third));

    /*
     * An empty query reports a complete inventory scan
     * with no active filters.
     */
    const auto unfiltered =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort = {},
                .offset = 0U,
                .limit = 2U});

    REQUIRE(
        unfiltered.query_diagnostics.active_filters ==
        0U);
    REQUIRE(
        unfiltered.query_diagnostics
            .subscriptions_scanned == 3U);
    REQUIRE(
        unfiltered.query_diagnostics
            .subscriptions_matched == 3U);
    REQUIRE(
        !unfiltered.query_diagnostics
             .effective_query.topic.has_value());

    /*
     * Every populated typed field contributes one active
     * logical-AND filter.
     */
    const SubscriptionInventoryQuery query{
        .topic = "diagnostics.alpha",
        .category = EventCategory::Runtime,
        .minimum_priority =
            EventPriority::Critical,
        .security_clearance =
            EventSecurity::Confidential,
        .enabled = true,
        .handler_available = true};

    const auto filtered =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = query,
                .sort = {},
                .offset = 0U,
                .limit = 1U});

    REQUIRE(filtered.entries.size() == 1U);
    REQUIRE(filtered.total_matches == 1U);

    REQUIRE(
        filtered.query_diagnostics.active_filters ==
        6U);
    REQUIRE(
        filtered.query_diagnostics
            .subscriptions_scanned == 3U);
    REQUIRE(
        filtered.query_diagnostics
            .subscriptions_matched == 1U);

    const auto& effective =
        filtered.query_diagnostics.effective_query;

    REQUIRE(effective.topic.has_value());
    REQUIRE(
        *effective.topic ==
        "diagnostics.alpha");
    REQUIRE(
        effective.category ==
        EventCategory::Runtime);
    REQUIRE(
        effective.minimum_priority ==
        EventPriority::Critical);
    REQUIRE(
        effective.security_clearance ==
        EventSecurity::Confidential);
    REQUIRE(effective.enabled == true);
    REQUIRE(effective.handler_available == true);

    /*
     * Metadata remains available on zero-limit and
     * no-match pages.
     */
    const auto metadata_only =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query =
                    SubscriptionInventoryQuery{
                        .topic =
                            "diagnostics.alpha"},
                .sort = {},
                .offset = 0U,
                .limit = 0U});

    REQUIRE(metadata_only.entries.empty());
    REQUIRE(
        metadata_only.query_diagnostics
            .active_filters == 1U);
    REQUIRE(
        metadata_only.query_diagnostics
            .subscriptions_scanned == 3U);
    REQUIRE(
        metadata_only.query_diagnostics
            .subscriptions_matched == 2U);

    const auto no_matches =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query =
                    SubscriptionInventoryQuery{
                        .topic =
                            "diagnostics.missing"},
                .sort = {},
                .offset = 0U,
                .limit = 4U});

    REQUIRE(no_matches.entries.empty());
    REQUIRE(
        no_matches.query_diagnostics
            .subscriptions_scanned == 3U);
    REQUIRE(
        no_matches.query_diagnostics
            .subscriptions_matched == 0U);

    /*
     * Diagnostics are detached from the request and from
     * future page results.
     */
    auto detached =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query =
                    SubscriptionInventoryQuery{
                        .topic =
                            "diagnostics.alpha"},
                .sort = {},
                .offset = 0U,
                .limit = 1U});

    detached.query_diagnostics
        .effective_query.topic =
            "mutated.copy";

    const auto unchanged =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query =
                    SubscriptionInventoryQuery{
                        .topic =
                            "diagnostics.alpha"},
                .sort = {},
                .offset = 0U,
                .limit = 1U});

    REQUIRE(
        unchanged.query_diagnostics
            .effective_query.topic.has_value());
    REQUIRE(
        *unchanged.query_diagnostics
             .effective_query.topic ==
        "diagnostics.alpha");

    return 0;
}
