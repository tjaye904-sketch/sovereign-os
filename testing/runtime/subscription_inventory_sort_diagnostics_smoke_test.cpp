#include "event_bus.hpp"

#include <cstddef>
#include <cstdint>
#include "test_require.hpp"

using namespace sovereign;

namespace
{

void AssertTerm(
    const SubscriptionInventorySortTerm& term,
    const SubscriptionInventorySortField field,
    const SubscriptionInventorySortDirection direction)
{
    REQUIRE(term.field == field);
    REQUIRE(term.direction == direction);
}

} // namespace

int main()
{
    EventBus bus;

    REQUIRE(
        bus.Subscribe(
            EventSubscription{
                .topic = "beta",
                .category = EventCategory::Runtime,
                .minimum_priority = EventPriority::Normal,
                .security_clearance = EventSecurity::Internal,
                .retry_policy = {},
                .handler = [](const Event&) {}}));

    REQUIRE(
        bus.Subscribe(
            EventSubscription{
                .topic = "alpha",
                .category = EventCategory::Security,
                .minimum_priority = EventPriority::High,
                .security_clearance =
                    EventSecurity::Restricted,
                .retry_policy = {},
                .handler = [](const Event&) {}}));

    REQUIRE(
        bus.Subscribe(
            EventSubscription{
                .topic = "alpha",
                .category = EventCategory::Runtime,
                .minimum_priority = EventPriority::Critical,
                .security_clearance =
                    EventSecurity::Confidential,
                .retry_policy = {},
                .handler = [](const Event&) {}}));

    /*
     * Default page requests use the RT-025 compatibility
     * fields and report their effective inventory ordering.
     */
    const auto default_page =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{});

    REQUIRE(
        default_page.sort_diagnostics
            .effective_terms.size() == 1U);

    REQUIRE(
        default_page.sort_diagnostics
            .ignored_terms == 0U);

    REQUIRE(
        default_page.sort_diagnostics
            .used_legacy_fields);

    REQUIRE(
        !default_page.sort_diagnostics
             .used_fallback);

    AssertTerm(
        default_page.sort_diagnostics
            .effective_terms[0U],
        SubscriptionInventorySortField::InventoryIndex,
        SubscriptionInventorySortDirection::Ascending);

    /*
     * A valid legacy specification is reported exactly.
     */
    const auto legacy_page =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort =
                    SubscriptionInventorySort{
                        .field =
                            SubscriptionInventorySortField::
                                Topic,
                        .direction =
                            SubscriptionInventorySortDirection::
                                Descending,
                        .terms = {}},
                .offset = 0U,
                .limit = 64U});

    REQUIRE(
        legacy_page.sort_diagnostics
            .effective_terms.size() == 1U);

    REQUIRE(
        legacy_page.sort_diagnostics
            .ignored_terms == 0U);

    REQUIRE(
        legacy_page.sort_diagnostics
            .used_legacy_fields);

    REQUIRE(
        !legacy_page.sort_diagnostics
             .used_fallback);

    AssertTerm(
        legacy_page.sort_diagnostics
            .effective_terms[0U],
        SubscriptionInventorySortField::Topic,
        SubscriptionInventorySortDirection::Descending);

    /*
     * Explicit terms report their normalized effective
     * sequence. The first field occurrence wins.
     */
    const auto normalized_page =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort =
                    SubscriptionInventorySort{
                        .terms =
                            {
                                {
                                    SubscriptionInventorySortField::
                                        Topic,
                                    SubscriptionInventorySortDirection::
                                        Ascending
                                },
                                {
                                    SubscriptionInventorySortField::
                                        Topic,
                                    SubscriptionInventorySortDirection::
                                        Descending
                                },
                                {
                                    static_cast<
                                        SubscriptionInventorySortField>(
                                            255U),
                                    SubscriptionInventorySortDirection::
                                        Ascending
                                },
                                {
                                    SubscriptionInventorySortField::
                                        MinimumPriority,
                                    SubscriptionInventorySortDirection::
                                        Descending
                                }
                            }},
                .offset = 0U,
                .limit = 64U});

    REQUIRE(
        normalized_page.sort_diagnostics
            .effective_terms.size() == 2U);

    REQUIRE(
        normalized_page.sort_diagnostics
            .ignored_terms == 2U);

    REQUIRE(
        !normalized_page.sort_diagnostics
             .used_legacy_fields);

    REQUIRE(
        !normalized_page.sort_diagnostics
             .used_fallback);

    AssertTerm(
        normalized_page.sort_diagnostics
            .effective_terms[0U],
        SubscriptionInventorySortField::Topic,
        SubscriptionInventorySortDirection::Ascending);

    AssertTerm(
        normalized_page.sort_diagnostics
            .effective_terms[1U],
        SubscriptionInventorySortField::MinimumPriority,
        SubscriptionInventorySortDirection::Descending);

    REQUIRE(normalized_page.entries.size() == 3U);
    REQUIRE(normalized_page.entries[0U].index == 2U);
    REQUIRE(normalized_page.entries[1U].index == 1U);
    REQUIRE(normalized_page.entries[2U].index == 0U);

    /*
     * A malformed-only explicit specification reports the
     * deterministic inventory-index fallback.
     */
    const auto fallback_page =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort =
                    SubscriptionInventorySort{
                        .terms =
                            {
                                {
                                    static_cast<
                                        SubscriptionInventorySortField>(
                                            200U),
                                    static_cast<
                                        SubscriptionInventorySortDirection>(
                                            200U)
                                }
                            }},
                .offset = 0U,
                .limit = 64U});

    REQUIRE(
        fallback_page.sort_diagnostics
            .effective_terms.size() == 1U);

    REQUIRE(
        fallback_page.sort_diagnostics
            .ignored_terms == 1U);

    REQUIRE(
        !fallback_page.sort_diagnostics
             .used_legacy_fields);

    REQUIRE(
        fallback_page.sort_diagnostics
            .used_fallback);

    AssertTerm(
        fallback_page.sort_diagnostics
            .effective_terms[0U],
        SubscriptionInventorySortField::InventoryIndex,
        SubscriptionInventorySortDirection::Ascending);

    REQUIRE(fallback_page.entries.size() == 3U);
    REQUIRE(fallback_page.entries[0U].index == 0U);
    REQUIRE(fallback_page.entries[1U].index == 1U);
    REQUIRE(fallback_page.entries[2U].index == 2U);

    /*
     * Diagnostics remain available on metadata-only pages.
     */
    const auto zero_limit =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort =
                    SubscriptionInventorySort{
                        .terms =
                            {
                                {
                                    SubscriptionInventorySortField::
                                        Category,
                                    SubscriptionInventorySortDirection::
                                        Descending
                                }
                            }},
                .offset = 0U,
                .limit = 0U});

    REQUIRE(zero_limit.entries.empty());

    REQUIRE(
        zero_limit.sort_diagnostics
            .effective_terms.size() == 1U);

    AssertTerm(
        zero_limit.sort_diagnostics
            .effective_terms[0U],
        SubscriptionInventorySortField::Category,
        SubscriptionInventorySortDirection::Descending);

    /*
     * Returned diagnostic metadata is detached value state.
     */
    auto detached =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort =
                    SubscriptionInventorySort{
                        .terms =
                            {
                                {
                                    SubscriptionInventorySortField::
                                        Topic,
                                    SubscriptionInventorySortDirection::
                                        Ascending
                                }
                            }},
                .offset = 0U,
                .limit = 1U});

    REQUIRE(
        detached.sort_diagnostics
            .effective_terms.size() == 1U);

    detached.sort_diagnostics
        .effective_terms[0U]
        .field =
            SubscriptionInventorySortField::
                HandlerAvailable;

    detached.sort_diagnostics
        .effective_terms.clear();

    detached.sort_diagnostics
        .ignored_terms = 999U;

    detached.sort_diagnostics
        .used_legacy_fields = true;

    detached.sort_diagnostics
        .used_fallback = true;

    const auto unchanged =
        bus.QuerySubscriptionInventoryPage(
            SubscriptionInventoryPageRequest{
                .query = {},
                .sort =
                    SubscriptionInventorySort{
                        .terms =
                            {
                                {
                                    SubscriptionInventorySortField::
                                        Topic,
                                    SubscriptionInventorySortDirection::
                                        Ascending
                                }
                            }},
                .offset = 0U,
                .limit = 1U});

    REQUIRE(
        unchanged.sort_diagnostics
            .effective_terms.size() == 1U);

    REQUIRE(
        unchanged.sort_diagnostics
            .ignored_terms == 0U);

    REQUIRE(
        !unchanged.sort_diagnostics
             .used_legacy_fields);

    REQUIRE(
        !unchanged.sort_diagnostics
             .used_fallback);

    AssertTerm(
        unchanged.sort_diagnostics
            .effective_terms[0U],
        SubscriptionInventorySortField::Topic,
        SubscriptionInventorySortDirection::Ascending);

    return 0;
}
