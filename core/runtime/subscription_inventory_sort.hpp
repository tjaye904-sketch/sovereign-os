#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace sovereign
{

enum class SubscriptionInventorySortField : std::uint8_t
{
    InventoryIndex = 0,
    Topic,
    Category,
    MinimumPriority,
    SecurityClearance,
    Enabled,
    HandlerAvailable
};

enum class SubscriptionInventorySortDirection : std::uint8_t
{
    Ascending = 0,
    Descending
};

inline constexpr std::size_t
    SubscriptionInventorySortFieldCount{7U};

struct SubscriptionInventorySortTerm final
{
    SubscriptionInventorySortField field{
        SubscriptionInventorySortField::InventoryIndex};

    SubscriptionInventorySortDirection direction{
        SubscriptionInventorySortDirection::Ascending};
};

struct SubscriptionInventorySort final
{
    /*
     * RT-025 compatibility fields.
     *
     * These fields are used when terms is empty.
     */
    SubscriptionInventorySortField field{
        SubscriptionInventorySortField::InventoryIndex};

    SubscriptionInventorySortDirection direction{
        SubscriptionInventorySortDirection::Ascending};

    /*
     * Ordered RT-026 sort terms.
     *
     * When non-empty, these terms replace the legacy
     * single-field sort above.
     */
    std::vector<SubscriptionInventorySortTerm> terms;
};

struct NormalizedSubscriptionInventorySort final
{
    std::vector<SubscriptionInventorySortTerm> terms;

    std::size_t ignored_terms{0U};

    bool used_legacy_fields{false};

    bool used_fallback{false};
};

[[nodiscard]]
constexpr bool IsValidSubscriptionInventorySortField(
    const SubscriptionInventorySortField field) noexcept
{
    switch (field)
    {
        case SubscriptionInventorySortField::InventoryIndex:
        case SubscriptionInventorySortField::Topic:
        case SubscriptionInventorySortField::Category:
        case SubscriptionInventorySortField::MinimumPriority:
        case SubscriptionInventorySortField::SecurityClearance:
        case SubscriptionInventorySortField::Enabled:
        case SubscriptionInventorySortField::HandlerAvailable:
            return true;
    }

    return false;
}

[[nodiscard]]
constexpr bool IsValidSubscriptionInventorySortDirection(
    const SubscriptionInventorySortDirection direction) noexcept
{
    switch (direction)
    {
        case SubscriptionInventorySortDirection::Ascending:
        case SubscriptionInventorySortDirection::Descending:
            return true;
    }

    return false;
}

[[nodiscard]]
constexpr std::size_t SubscriptionInventorySortFieldIndex(
    const SubscriptionInventorySortField field) noexcept
{
    return static_cast<std::size_t>(field);
}

[[nodiscard]]
inline NormalizedSubscriptionInventorySort
NormalizeSubscriptionInventorySort(
    const SubscriptionInventorySort& specification)
{
    NormalizedSubscriptionInventorySort normalized;

    normalized.terms.reserve(
        SubscriptionInventorySortFieldCount);

    if (specification.terms.empty())
    {
        normalized.used_legacy_fields = true;

        if (IsValidSubscriptionInventorySortField(
                specification.field) &&
            IsValidSubscriptionInventorySortDirection(
                specification.direction))
        {
            normalized.terms.push_back(
                SubscriptionInventorySortTerm{
                    .field = specification.field,
                    .direction = specification.direction});

            return normalized;
        }

        normalized.ignored_terms = 1U;
        normalized.used_fallback = true;

        normalized.terms.push_back(
            SubscriptionInventorySortTerm{
                .field =
                    SubscriptionInventorySortField::
                        InventoryIndex,
                .direction =
                    SubscriptionInventorySortDirection::
                        Ascending});

        return normalized;
    }

    std::array<bool, SubscriptionInventorySortFieldCount>
        fields_seen{};

    for (const SubscriptionInventorySortTerm& term :
         specification.terms)
    {
        if (!IsValidSubscriptionInventorySortField(
                term.field) ||
            !IsValidSubscriptionInventorySortDirection(
                term.direction))
        {
            ++normalized.ignored_terms;
            continue;
        }

        const std::size_t field_index =
            SubscriptionInventorySortFieldIndex(
                term.field);

        if (fields_seen[field_index])
        {
            ++normalized.ignored_terms;
            continue;
        }

        fields_seen[field_index] = true;
        normalized.terms.push_back(term);
    }

    if (!normalized.terms.empty())
    {
        return normalized;
    }

    normalized.used_fallback = true;

    normalized.terms.push_back(
        SubscriptionInventorySortTerm{
            .field =
                SubscriptionInventorySortField::
                    InventoryIndex,
            .direction =
                SubscriptionInventorySortDirection::
                    Ascending});

    return normalized;
}

} // namespace sovereign
