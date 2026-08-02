#include "event_bus.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>
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

void AssertIndices(
    const SubscriptionInventoryPage& page,
    const std::vector<std::size_t>& expected)
{
    REQUIRE(page.entries.size() == expected.size());

    for (std::size_t index = 0U;
         index < expected.size();
         ++index)
    {
        REQUIRE(page.entries[index].index == expected[index]);
    }
}

} // namespace

int main()
{
    /*
     * Empty explicit terms preserve RT-025 compatibility.
     */
    const auto legacy =
        NormalizeSubscriptionInventorySort(
            SubscriptionInventorySort{
                .field =
                    SubscriptionInventorySortField::Topic,
                .direction =
                    SubscriptionInventorySortDirection::
                        Descending,
                .terms = {}});

    REQUIRE(legacy.used_legacy_fields);
    REQUIRE(!legacy.used_fallback);
    REQUIRE(legacy.ignored_terms == 0U);
    REQUIRE(legacy.terms.size() == 1U);

    AssertTerm(
        legacy.terms[0U],
        SubscriptionInventorySortField::Topic,
        SubscriptionInventorySortDirection::Descending);

    /*
     * First occurrence of a field wins. Later occurrences
     * are ignored regardless of direction.
     */
    const auto duplicates =
        NormalizeSubscriptionInventorySort(
            SubscriptionInventorySort{
                .terms =
                    {
                        SubscriptionInventorySortTerm{
                            .field =
                                SubscriptionInventorySortField::
                                    Topic,
                            .direction =
                                SubscriptionInventorySortDirection::
                                    Ascending},
                        SubscriptionInventorySortTerm{
                            .field =
                                SubscriptionInventorySortField::
                                    Category,
                            .direction =
                                SubscriptionInventorySortDirection::
                                    Descending},
                        SubscriptionInventorySortTerm{
                            .field =
                                SubscriptionInventorySortField::
                                    Topic,
                            .direction =
                                SubscriptionInventorySortDirection::
                                    Descending}
                    }});

    REQUIRE(!duplicates.used_legacy_fields);
    REQUIRE(!duplicates.used_fallback);
    REQUIRE(duplicates.ignored_terms == 1U);
    REQUIRE(duplicates.terms.size() == 2U);

    AssertTerm(
        duplicates.terms[0U],
        SubscriptionInventorySortField::Topic,
        SubscriptionInventorySortDirection::Ascending);

    AssertTerm(
        duplicates.terms[1U],
        SubscriptionInventorySortField::Category,
        SubscriptionInventorySortDirection::Descending);

    /*
     * Malformed fields and directions are ignored.
     */
    const auto malformed =
        NormalizeSubscriptionInventorySort(
            SubscriptionInventorySort{
                .terms =
                    {
                        SubscriptionInventorySortTerm{
                            .field =
                                static_cast<
                                    SubscriptionInventorySortField>(
                                        255U),
                            .direction =
                                SubscriptionInventorySortDirection::
                                    Ascending},
                        SubscriptionInventorySortTerm{
                            .field =
                                SubscriptionInventorySortField::
                                    Enabled,
                            .direction =
                                static_cast<
                                    SubscriptionInventorySortDirection>(
                                        255U)},
                        SubscriptionInventorySortTerm{
                            .field =
                                SubscriptionInventorySortField::
                                    SecurityClearance,
                            .direction =
                                SubscriptionInventorySortDirection::
                                    Descending}
                    }});

    REQUIRE(!malformed.used_legacy_fields);
    REQUIRE(!malformed.used_fallback);
    REQUIRE(malformed.ignored_terms == 2U);
    REQUIRE(malformed.terms.size() == 1U);

    AssertTerm(
        malformed.terms[0U],
        SubscriptionInventorySortField::SecurityClearance,
        SubscriptionInventorySortDirection::Descending);

    /*
     * A completely unusable explicit specification receives
     * a deterministic inventory-index fallback.
     */
    const auto fallback =
        NormalizeSubscriptionInventorySort(
            SubscriptionInventorySort{
                .terms =
                    {
                        SubscriptionInventorySortTerm{
                            .field =
                                static_cast<
                                    SubscriptionInventorySortField>(
                                        200U),
                            .direction =
                                static_cast<
                                    SubscriptionInventorySortDirection>(
                                        200U)}
                    }});

    REQUIRE(!fallback.used_legacy_fields);
    REQUIRE(fallback.used_fallback);
    REQUIRE(fallback.ignored_terms == 1U);
    REQUIRE(fallback.terms.size() == 1U);

    AssertTerm(
        fallback.terms[0U],
        SubscriptionInventorySortField::InventoryIndex,
        SubscriptionInventorySortDirection::Ascending);

    /*
     * The seven unique supported fields establish the hard
     * upper bound for normalized sort complexity.
     */
    const auto bounded =
        NormalizeSubscriptionInventorySort(
            SubscriptionInventorySort{
                .terms =
                    {
                        {
                            SubscriptionInventorySortField::
                                HandlerAvailable,
                            SubscriptionInventorySortDirection::
                                Ascending
                        },
                        {
                            SubscriptionInventorySortField::
                                Enabled,
                            SubscriptionInventorySortDirection::
                                Descending
                        },
                        {
                            SubscriptionInventorySortField::
                                SecurityClearance,
                            SubscriptionInventorySortDirection::
                                Ascending
                        },
                        {
                            SubscriptionInventorySortField::
                                MinimumPriority,
                            SubscriptionInventorySortDirection::
                                Descending
                        },
                        {
                            SubscriptionInventorySortField::
                                Category,
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
                            SubscriptionInventorySortField::
                                InventoryIndex,
                            SubscriptionInventorySortDirection::
                                Ascending
                        },
                        {
                            SubscriptionInventorySortField::
                                Topic,
                            SubscriptionInventorySortDirection::
                                Ascending
                        }
                    }});

    REQUIRE(
        bounded.terms.size() ==
        SubscriptionInventorySortFieldCount);

    REQUIRE(bounded.ignored_terms == 1U);

    /*
     * Runtime integration uses the normalized specification.
     */
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
                .security_clearance = EventSecurity::Restricted,
                .retry_policy = {},
                .handler = [](const Event&) {}}));

    REQUIRE(
        bus.Subscribe(
            EventSubscription{
                .topic = "alpha",
                .category = EventCategory::Runtime,
                .minimum_priority = EventPriority::Critical,
                .security_clearance = EventSecurity::Public,
                .retry_policy = {},
                .handler = [](const Event&) {}}));

    /*
     * The duplicate descending topic term is ignored because
     * the first topic term is ascending.
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
                                    SubscriptionInventorySortField::
                                        MinimumPriority,
                                    SubscriptionInventorySortDirection::
                                        Descending
                                }
                            }},
                .offset = 0U,
                .limit = 64U});

    AssertIndices(normalized_page, {2U, 1U, 0U});

    /*
     * A malformed-only specification falls back to stable
     * inventory order rather than invoking undefined enum
     * ordering behavior.
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
                                            255U),
                                    static_cast<
                                        SubscriptionInventorySortDirection>(
                                            255U)
                                }
                            }},
                .offset = 0U,
                .limit = 64U});

    AssertIndices(fallback_page, {0U, 1U, 2U});

    return 0;
}
