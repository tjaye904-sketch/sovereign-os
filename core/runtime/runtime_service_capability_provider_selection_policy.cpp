#include "runtime_service_capability_provider_selection_policy.hpp"

#include "service_state.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace sovereign
{

bool RuntimeServiceCapabilityProviderSelectionPolicy::
SetProviderRank(
    std::string capability,
    std::string service_name,
    const std::uint32_t rank)
{
    if (capability.empty() ||
        service_name.empty())
    {
        return false;
    }

    const auto existing =
        std::find_if(
            provider_ranks_.begin(),
            provider_ranks_.end(),
            [&capability, &service_name](
                const ProviderRank& provider_rank)
            {
                return
                    provider_rank.capability ==
                        capability &&
                    provider_rank.service_name ==
                        service_name;
            });

    if (existing != provider_ranks_.end())
    {
        existing->rank = rank;
        return true;
    }

    provider_ranks_.push_back(
        ProviderRank{
            .capability = std::move(capability),
            .service_name =
                std::move(service_name),
            .rank = rank});

    return true;
}

bool RuntimeServiceCapabilityProviderSelectionPolicy::
RemoveProviderRank(
    const std::string_view capability,
    const std::string_view service_name)
{
    const auto existing =
        std::find_if(
            provider_ranks_.begin(),
            provider_ranks_.end(),
            [capability, service_name](
                const ProviderRank& provider_rank)
            {
                return
                    provider_rank.capability ==
                        capability &&
                    provider_rank.service_name ==
                        service_name;
            });

    if (existing == provider_ranks_.end())
    {
        return false;
    }

    provider_ranks_.erase(existing);
    return true;
}

void RuntimeServiceCapabilityProviderSelectionPolicy::
SetPreferredProviderFallbackEnabled(
    const bool enabled) noexcept
{
    preferred_provider_fallback_enabled_ =
        enabled;
}

bool RuntimeServiceCapabilityProviderSelectionPolicy::
PreferredProviderFallbackEnabled() const noexcept
{
    return
        preferred_provider_fallback_enabled_;
}

const RuntimeServiceCapabilityProviderSelectionPolicy::
ProviderRank*
RuntimeServiceCapabilityProviderSelectionPolicy::
FindProviderRank(
    const std::string_view capability,
    const std::string_view service_name) const
    noexcept
{
    const auto provider_rank =
        std::find_if(
            provider_ranks_.begin(),
            provider_ranks_.end(),
            [capability, service_name](
                const ProviderRank& candidate)
            {
                return
                    candidate.capability ==
                        capability &&
                    candidate.service_name ==
                        service_name;
            });

    if (provider_rank == provider_ranks_.end())
    {
        return nullptr;
    }

    return &*provider_rank;
}

RuntimeServiceCapabilityProviderSelectionResult
RuntimeServiceCapabilityProviderSelectionPolicy::
Select(
    const
    RuntimeServiceCapabilityProviderSelectionRequest&
        request,
    const std::vector<ServiceCapabilityProvider>&
        providers)
{
    RuntimeServiceCapabilityProviderSelectionResult
        result{};

    result.candidate_count = providers.size();
    ++selection_count_;

    const auto eligible =
        [&request](
            const ServiceCapabilityProvider& provider)
        {
            return
                !request.require_running ||
                provider.service_state ==
                    ServiceState::Running;
        };

    result.eligible_count =
        static_cast<std::size_t>(
            std::count_if(
                providers.begin(),
                providers.end(),
                eligible));

    if (!request.preferred_service_name.empty())
    {
        const auto preferred =
            std::find_if(
                providers.begin(),
                providers.end(),
                [&request, &eligible](
                    const ServiceCapabilityProvider&
                        provider)
                {
                    return
                        provider.service_name ==
                            request
                                .preferred_service_name &&
                        eligible(provider);
                });

        if (preferred != providers.end())
        {
            result.status =
                RuntimeServiceCapabilityProviderSelectionStatus::
                    Selected;

            result.selected_provider = *preferred;
            result.used_preferred_provider = true;

            if (const auto* provider_rank =
                    FindProviderRank(
                        request.capability,
                        preferred->service_name);
                provider_rank != nullptr)
            {
                result.selected_rank =
                    provider_rank->rank;

                result.used_rank = true;
            }

            ++preferred_selection_count_;
            return result;
        }

        if (!preferred_provider_fallback_enabled_)
        {
            result.status =
                RuntimeServiceCapabilityProviderSelectionStatus::
                    PreferredProviderUnavailable;

            result.diagnostic =
                "The preferred runtime service "
                "provider is unavailable.";

            return result;
        }

        result.used_preferred_fallback = true;
    }

    const ServiceCapabilityProvider* selected{
        nullptr};

    const ProviderRank* selected_rank{nullptr};

    for (const auto& provider : providers)
    {
        if (!eligible(provider))
        {
            continue;
        }

        const ProviderRank* provider_rank =
            FindProviderRank(
                request.capability,
                provider.service_name);

        if (selected == nullptr)
        {
            selected = &provider;
            selected_rank = provider_rank;
            continue;
        }

        const std::uint32_t candidate_rank =
            provider_rank == nullptr
                ? std::numeric_limits<
                      std::uint32_t>::max()
                : provider_rank->rank;

        const std::uint32_t current_rank =
            selected_rank == nullptr
                ? std::numeric_limits<
                      std::uint32_t>::max()
                : selected_rank->rank;

        if (candidate_rank < current_rank ||
            (candidate_rank == current_rank &&
             provider.service_ordinal <
                 selected->service_ordinal))
        {
            selected = &provider;
            selected_rank = provider_rank;
        }
    }

    if (selected == nullptr)
    {
        result.status =
            RuntimeServiceCapabilityProviderSelectionStatus::
                NoEligibleProvider;

        result.diagnostic =
            "No eligible runtime service provider "
            "is available.";

        return result;
    }

    result.status =
        RuntimeServiceCapabilityProviderSelectionStatus::
            Selected;

    result.selected_provider = *selected;

    if (selected_rank != nullptr)
    {
        result.selected_rank =
            selected_rank->rank;

        result.used_rank = true;
        ++ranked_selection_count_;
    }

    if (result.used_preferred_fallback)
    {
        ++preferred_fallback_count_;
    }

    return result;
}

std::vector<
    RuntimeServiceCapabilityProviderRankEntry>
RuntimeServiceCapabilityProviderSelectionPolicy::
ProviderRanks() const
{
    std::vector<
        RuntimeServiceCapabilityProviderRankEntry>
            provider_ranks{};

    provider_ranks.reserve(
        provider_ranks_.size());

    for (const auto& provider_rank :
         provider_ranks_)
    {
        provider_ranks.push_back(
            RuntimeServiceCapabilityProviderRankEntry{
                .capability =
                    provider_rank.capability,
                .service_name =
                    provider_rank.service_name,
                .rank =
                    provider_rank.rank});
    }

    return provider_ranks;
}

std::size_t
RuntimeServiceCapabilityProviderSelectionPolicy::
ProviderRankCount() const noexcept
{
    return provider_ranks_.size();
}

std::uint64_t
RuntimeServiceCapabilityProviderSelectionPolicy::
SelectionCount() const noexcept
{
    return selection_count_;
}

std::uint64_t
RuntimeServiceCapabilityProviderSelectionPolicy::
RankedSelectionCount() const noexcept
{
    return ranked_selection_count_;
}

std::uint64_t
RuntimeServiceCapabilityProviderSelectionPolicy::
PreferredSelectionCount() const noexcept
{
    return preferred_selection_count_;
}

std::uint64_t
RuntimeServiceCapabilityProviderSelectionPolicy::
PreferredFallbackCount() const noexcept
{
    return preferred_fallback_count_;
}

} // namespace sovereign
