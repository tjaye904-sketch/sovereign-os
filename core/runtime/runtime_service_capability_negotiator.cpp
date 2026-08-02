#include "runtime_service_capability_negotiator.hpp"

#include "runtime_service_capability_provider_selection_policy.hpp"

#include <algorithm>
#include <utility>

namespace sovereign
{

RuntimeServiceCapabilityNegotiator::
RuntimeServiceCapabilityNegotiator(
    const ServiceRegistry& registry) noexcept
    :
    registry_(registry),
    discovery_(registry)
{
}

RuntimeServiceCapabilityNegotiator::
RuntimeServiceCapabilityNegotiator(
    const ServiceRegistry& registry,
    RuntimeServiceCapabilityProviderSelectionPolicy&
        selection_policy) noexcept
    :
    registry_(registry),
    discovery_(registry),
    selection_policy_(&selection_policy)
{
}

RuntimeServiceCapabilityResult
RuntimeServiceCapabilityNegotiator::Negotiate(
    RuntimeServiceCapabilityRequest request)
{
    RuntimeServiceCapabilityResult result{};

    result.capability =
        std::move(request.capability);

    result.registry_generation =
        registry_.Generation();

    result.negotiation_sequence =
        ++negotiation_count_;

    if (result.capability.empty())
    {
        result.status =
            RuntimeServiceCapabilityStatus::
                EmptyCapability;

        result.diagnostic =
            "Runtime service capability "
            "negotiation requires a capability.";

        last_negotiation_ = result;
        return result;
    }

    const auto providers =
        discovery_.Providers(result.capability);

    result.candidate_count = providers.size();

    if (providers.empty())
    {
        result.status =
            RuntimeServiceCapabilityStatus::
                CapabilityNotFound;

        result.diagnostic =
            "No registered runtime service "
            "provides the requested capability.";

        last_negotiation_ = result;
        return result;
    }

    const auto eligible =
        [&request](
            const ServiceCapabilityProvider&
                provider)
        {
            return
                !request.require_running ||
                provider.service_state ==
                    ServiceState::Running;
        };

    const ServiceCapabilityProvider*
        selected{nullptr};

    ServiceCapabilityProvider
        policy_selected_provider{};

    if (selection_policy_ != nullptr)
    {
        const auto selection =
            selection_policy_->Select(
                RuntimeServiceCapabilityProviderSelectionRequest{
                    .capability =
                        result.capability,
                    .preferred_service_name =
                        request.preferred_service_name,
                    .require_running =
                        request.require_running},
                providers);

        if (!selection.Selected())
        {
            if (selection.status ==
                RuntimeServiceCapabilityProviderSelectionStatus::
                    PreferredProviderUnavailable)
            {
                result.status =
                    RuntimeServiceCapabilityStatus::
                        PreferredProviderUnavailable;
            }
            else
            {
                result.status =
                    RuntimeServiceCapabilityStatus::
                        NoEligibleProvider;
            }

            result.diagnostic =
                selection.diagnostic;

            last_negotiation_ = result;
            return result;
        }

        policy_selected_provider =
            selection.selected_provider;

        selected = &policy_selected_provider;
    }
    else if (!request.preferred_service_name.empty())
    {
        const auto preferred =
            std::find_if(
                providers.begin(),
                providers.end(),
                [&request](
                    const ServiceCapabilityProvider&
                        provider)
                {
                    return
                        provider.service_name ==
                        request.preferred_service_name;
                });

        if (preferred == providers.end() ||
            !eligible(*preferred))
        {
            result.status =
                RuntimeServiceCapabilityStatus::
                    PreferredProviderUnavailable;

            result.diagnostic =
                "The preferred runtime service "
                "provider is unavailable.";

            last_negotiation_ = result;
            return result;
        }

        selected = &*preferred;
    }
    else
    {
        const auto provider =
            std::find_if(
                providers.begin(),
                providers.end(),
                eligible);

        if (provider == providers.end())
        {
            result.status =
                RuntimeServiceCapabilityStatus::
                    NoEligibleProvider;

            result.diagnostic =
                "No eligible runtime service "
                "provider is available.";

            last_negotiation_ = result;
            return result;
        }

        selected = &*provider;
    }

    result.status =
        RuntimeServiceCapabilityStatus::Accepted;

    result.selected_service_name =
        selected->service_name;

    result.selected_service_ordinal =
        selected->service_ordinal;

    result.selected_service_state =
        selected->service_state;

    ++accepted_count_;

    last_negotiation_ = result;
    return result;
}

const RuntimeServiceCapabilityResult&
RuntimeServiceCapabilityNegotiator::
LastNegotiation() const noexcept
{
    return last_negotiation_;
}

std::uint64_t
RuntimeServiceCapabilityNegotiator::
NegotiationCount() const noexcept
{
    return negotiation_count_;
}

std::uint64_t
RuntimeServiceCapabilityNegotiator::
AcceptedCount() const noexcept
{
    return accepted_count_;
}

} // namespace sovereign
