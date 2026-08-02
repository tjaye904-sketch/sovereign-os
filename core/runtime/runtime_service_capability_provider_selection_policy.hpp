#pragma once

#include "service_capability.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sovereign
{

enum class
RuntimeServiceCapabilityProviderSelectionStatus :
    std::uint8_t
{
    NoAttempt = 0U,
    Selected,
    NoEligibleProvider,
    PreferredProviderUnavailable
};

struct
RuntimeServiceCapabilityProviderSelectionRequest final
{
    std::string capability{};
    std::string preferred_service_name{};
    bool require_running{true};
};

struct
RuntimeServiceCapabilityProviderSelectionResult final
{
    RuntimeServiceCapabilityProviderSelectionStatus
        status{
            RuntimeServiceCapabilityProviderSelectionStatus::
                NoAttempt};

    ServiceCapabilityProvider selected_provider{};

    std::size_t candidate_count{0U};
    std::size_t eligible_count{0U};

    std::uint32_t selected_rank{0U};

    bool used_rank{false};
    bool used_preferred_provider{false};
    bool used_preferred_fallback{false};

    std::string diagnostic{};

    [[nodiscard]]
    bool Selected() const noexcept
    {
        return
            status ==
            RuntimeServiceCapabilityProviderSelectionStatus::
                Selected;
    }
};


struct
RuntimeServiceCapabilityProviderRankEntry final
{
    std::string capability{};
    std::string service_name{};
    std::uint32_t rank{0U};
};

class
RuntimeServiceCapabilityProviderSelectionPolicy final
{
public:
    bool SetProviderRank(
        std::string capability,
        std::string service_name,
        std::uint32_t rank);

    bool RemoveProviderRank(
        std::string_view capability,
        std::string_view service_name);

    void SetPreferredProviderFallbackEnabled(
        bool enabled) noexcept;

    [[nodiscard]]
    bool PreferredProviderFallbackEnabled()
        const noexcept;

    [[nodiscard]]
    RuntimeServiceCapabilityProviderSelectionResult
    Select(
        const
        RuntimeServiceCapabilityProviderSelectionRequest&
            request,
        const std::vector<ServiceCapabilityProvider>&
            providers);

    [[nodiscard]]
    std::vector<
        RuntimeServiceCapabilityProviderRankEntry>
    ProviderRanks() const;

    [[nodiscard]]
    std::size_t ProviderRankCount() const noexcept;

    [[nodiscard]]
    std::uint64_t SelectionCount() const noexcept;

    [[nodiscard]]
    std::uint64_t RankedSelectionCount()
        const noexcept;

    [[nodiscard]]
    std::uint64_t PreferredSelectionCount()
        const noexcept;

    [[nodiscard]]
    std::uint64_t PreferredFallbackCount()
        const noexcept;

private:
    struct ProviderRank final
    {
        std::string capability{};
        std::string service_name{};
        std::uint32_t rank{0U};
    };

    [[nodiscard]]
    const ProviderRank* FindProviderRank(
        std::string_view capability,
        std::string_view service_name) const
        noexcept;

    std::vector<ProviderRank> provider_ranks_{};

    bool preferred_provider_fallback_enabled_{
        false};

    std::uint64_t selection_count_{0U};
    std::uint64_t ranked_selection_count_{0U};
    std::uint64_t preferred_selection_count_{0U};
    std::uint64_t preferred_fallback_count_{0U};
};

} // namespace sovereign
