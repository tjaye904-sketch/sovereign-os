#pragma once

#include "subscription_inventory_query.hpp"
#include "subscription_inventory_sort.hpp"
#include "subscription_snapshot.hpp"

#include <cstddef>
#include <vector>

namespace sovereign
{

inline constexpr std::size_t
    DefaultSubscriptionInventoryPageLimit{64U};

struct SubscriptionInventoryPageRequest final
{
    SubscriptionInventoryQuery query{};

    SubscriptionInventorySort sort{};

    std::size_t offset{0U};

    std::size_t limit{
        DefaultSubscriptionInventoryPageLimit};
};

struct SubscriptionInventorySortDiagnostics final
{
    /*
     * Effective ordered sort terms after validation,
     * duplicate removal, and fallback processing.
     */
    std::vector<SubscriptionInventorySortTerm>
        effective_terms;

    /*
     * Requested terms discarded because they were invalid
     * or duplicated an earlier field.
     */
    std::size_t ignored_terms{0U};

    /*
     * True when the RT-025 compatibility fields supplied
     * the effective sort specification.
     */
    bool used_legacy_fields{false};

    /*
     * True when no usable requested sort remained and the
     * deterministic inventory-index fallback was applied.
     */
    bool used_fallback{false};
};

struct SubscriptionInventoryQueryDiagnostics final
{
    /*
     * Detached copy of the effective typed query.
     */
    SubscriptionInventoryQuery effective_query{};

    /*
     * Number of populated query fields combined using
     * logical AND.
     */
    std::size_t active_filters{0U};

    /*
     * Number of subscriptions evaluated by the query.
     */
    std::size_t subscriptions_scanned{0U};

    /*
     * Number of subscriptions satisfying the query before
     * pagination is applied.
     */
    std::size_t subscriptions_matched{0U};
};

struct SubscriptionInventoryExecutionDiagnostics final
{
    /*
     * Requested pagination coordinates.
     */
    std::size_t requested_offset{0U};

    std::size_t requested_limit{0U};

    /*
     * Result-shaping counters before and after pagination.
     */
    std::size_t matched_entries{0U};

    std::size_t returned_entries{0U};

    std::size_t remaining_entries{0U};

    /*
     * True when the requested offset identifies an
     * available matching entry.
     */
    bool offset_in_range{false};

    /*
     * True when the request intentionally returns only
     * metadata because its limit is zero.
     */
    bool metadata_only{false};

    /*
     * True when no later matching page remains.
     */
    bool terminal_page{false};
};

struct SubscriptionInventoryPage final
{
    std::vector<SubscriptionSnapshot> entries;

    std::size_t offset{0U};

    std::size_t limit{0U};

    std::size_t total_matches{0U};

    bool has_more{false};

    SubscriptionInventorySortDiagnostics
        sort_diagnostics{};

    SubscriptionInventoryQueryDiagnostics
        query_diagnostics{};

    SubscriptionInventoryExecutionDiagnostics
        execution_diagnostics{};
};

} // namespace sovereign
