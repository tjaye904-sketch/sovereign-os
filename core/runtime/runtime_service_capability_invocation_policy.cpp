#include "runtime_service_capability_invocation_policy.hpp"

#include <algorithm>
#include <utility>

namespace sovereign
{

bool RuntimeServiceCapabilityInvocationPolicy::
DenyCapability(std::string capability)
{
    if (capability.empty())
    {
        return false;
    }

    const auto found =
        std::find(
            denied_capabilities_.begin(),
            denied_capabilities_.end(),
            capability);

    if (found != denied_capabilities_.end())
    {
        return false;
    }

    denied_capabilities_.push_back(
        std::move(capability));

    return true;
}

bool RuntimeServiceCapabilityInvocationPolicy::
DenyOperation(
    std::string capability,
    std::string operation)
{
    if (capability.empty() || operation.empty())
    {
        return false;
    }

    const auto found =
        std::find_if(
            denied_operations_.begin(),
            denied_operations_.end(),
            [&capability, &operation](
                const auto& rule)
            {
                return
                    rule.capability == capability &&
                    rule.operation == operation;
            });

    if (found != denied_operations_.end())
    {
        return false;
    }

    denied_operations_.push_back(
        RuntimeServiceCapabilityOperationPolicyRule{
            .capability = std::move(capability),
            .operation = std::move(operation)});

    return true;
}

bool RuntimeServiceCapabilityInvocationPolicy::
DenyProvider(
    std::string capability,
    std::string provider_service_name)
{
    if (capability.empty() ||
        provider_service_name.empty())
    {
        return false;
    }

    const auto found =
        std::find_if(
            denied_providers_.begin(),
            denied_providers_.end(),
            [&capability, &provider_service_name](
                const auto& rule)
            {
                return
                    rule.capability == capability &&
                    rule.provider_service_name ==
                        provider_service_name;
            });

    if (found != denied_providers_.end())
    {
        return false;
    }

    denied_providers_.push_back(
        RuntimeServiceCapabilityProviderPolicyRule{
            .capability = std::move(capability),
            .provider_service_name =
                std::move(provider_service_name)});

    return true;
}

RuntimeServiceCapabilityInvocationPolicyDecision
RuntimeServiceCapabilityInvocationPolicy::
EvaluateRequest(
    const RuntimeServiceCapabilityInvocationRequest&
        request) const
{
    RuntimeServiceCapabilityInvocationPolicyDecision
        decision{};

    decision.capability = request.capability;
    decision.operation = request.operation;

    if (request.capability.empty())
    {
        decision.status =
            RuntimeServiceCapabilityInvocationPolicyStatus::
                EmptyCapability;

        decision.diagnostic =
            "Capability invocation policy rejected "
            "an empty capability.";

        return decision;
    }

    const auto denied_capability =
        std::find(
            denied_capabilities_.begin(),
            denied_capabilities_.end(),
            request.capability);

    if (denied_capability !=
        denied_capabilities_.end())
    {
        decision.status =
            RuntimeServiceCapabilityInvocationPolicyStatus::
                CapabilityDenied;

        decision.diagnostic =
            "Capability invocation policy denied "
            "the requested capability.";

        return decision;
    }

    const auto denied_operation =
        std::find_if(
            denied_operations_.begin(),
            denied_operations_.end(),
            [&request](const auto& rule)
            {
                return
                    rule.capability ==
                        request.capability &&
                    rule.operation ==
                        request.operation;
            });

    if (denied_operation != denied_operations_.end())
    {
        decision.status =
            RuntimeServiceCapabilityInvocationPolicyStatus::
                OperationDenied;

        decision.diagnostic =
            "Capability invocation policy denied "
            "the requested operation.";

        return decision;
    }

    decision.status =
        RuntimeServiceCapabilityInvocationPolicyStatus::
            Allowed;

    return decision;
}

RuntimeServiceCapabilityInvocationPolicyDecision
RuntimeServiceCapabilityInvocationPolicy::
EvaluateProvider(
    const RuntimeServiceCapabilityInvocationRequest&
        request,
    const std::string_view provider_service_name)
        const
{
    RuntimeServiceCapabilityInvocationPolicyDecision
        decision{};

    decision.capability = request.capability;
    decision.operation = request.operation;

    decision.provider_service_name =
        provider_service_name;

    const auto denied_provider =
        std::find_if(
            denied_providers_.begin(),
            denied_providers_.end(),
            [&request, provider_service_name](
                const auto& rule)
            {
                return
                    rule.capability ==
                        request.capability &&
                    rule.provider_service_name ==
                        provider_service_name;
            });

    if (denied_provider != denied_providers_.end())
    {
        decision.status =
            RuntimeServiceCapabilityInvocationPolicyStatus::
                ProviderDenied;

        decision.diagnostic =
            "Capability invocation policy denied "
            "the negotiated provider.";

        return decision;
    }

    decision.status =
        RuntimeServiceCapabilityInvocationPolicyStatus::
            Allowed;

    return decision;
}

std::size_t
RuntimeServiceCapabilityInvocationPolicy::
DeniedCapabilityCount() const noexcept
{
    return denied_capabilities_.size();
}

std::size_t
RuntimeServiceCapabilityInvocationPolicy::
DeniedOperationCount() const noexcept
{
    return denied_operations_.size();
}

std::size_t
RuntimeServiceCapabilityInvocationPolicy::
DeniedProviderCount() const noexcept
{
    return denied_providers_.size();
}

} // namespace sovereign
