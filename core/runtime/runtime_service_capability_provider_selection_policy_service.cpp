#include "runtime_service_capability_provider_selection_policy_service.hpp"

#include "runtime_service_capability_provider_selection_policy_protocol.hpp"

#include <utility>

namespace sovereign
{

RuntimeServiceCapabilityProviderSelectionPolicyService::
RuntimeServiceCapabilityProviderSelectionPolicyService(
    RuntimeServiceCapabilityProviderSelectionPolicy&
        policy) noexcept
    : policy_(policy)
{
}

std::string_view
RuntimeServiceCapabilityProviderSelectionPolicyService::
Name() const noexcept
{
    return ServiceName;
}

bool
RuntimeServiceCapabilityProviderSelectionPolicyService::
Start(EventBus&)
{
    return true;
}

bool
RuntimeServiceCapabilityProviderSelectionPolicyService::
Update(const FrameContext&)
{
    return true;
}

bool
RuntimeServiceCapabilityProviderSelectionPolicyService::
Stop()
{
    return true;
}

bool
RuntimeServiceCapabilityProviderSelectionPolicyService::
SupportsOperation(
    const std::string_view operation)
    const noexcept
{
    return operation == ApplyOperation;
}

bool
RuntimeServiceCapabilityProviderSelectionPolicyService::
Invoke(
    const std::string_view operation,
    const std::string_view request_payload,
    std::string& response_payload)
{
    if (!SupportsOperation(operation))
    {
        return false;
    }

    std::scoped_lock lock{mutex_};

    const auto request =
        RuntimeServiceCapabilityProviderSelectionPolicyProtocol::
            DecodeRequest(request_payload);

    RuntimeServiceCapabilityProviderSelectionPolicyProtocolResult
        result{};

    if (!request)
    {
        result =
            Snapshot(
                RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus::
                    InvalidPayload,
                "provider-selection policy request payload is invalid");
    }
    else
    {
        result = Execute(*request);
    }

    response_payload =
        RuntimeServiceCapabilityProviderSelectionPolicyProtocol::
            EncodeResult(result);

    return true;
}

RuntimeServiceCapabilityProviderSelectionPolicyProtocolResult
RuntimeServiceCapabilityProviderSelectionPolicyService::
Execute(
    const
    RuntimeServiceCapabilityProviderSelectionPolicyProtocolRequest&
        request)
{
    using Command =
        RuntimeServiceCapabilityProviderSelectionPolicyCommand;

    switch (request.command)
    {
        case Command::SetRank:
        {
            const bool changed =
                policy_.SetProviderRank(
                    request.capability,
                    request.service_name,
                    request.rank);

            return
                Snapshot(
                    RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus::
                        Accepted,
                    changed
                        ? "provider rank stored"
                        : "provider rank unchanged");
        }

        case Command::RemoveRank:
        {
            const bool removed =
                policy_.RemoveProviderRank(
                    request.capability,
                    request.service_name);

            return
                Snapshot(
                    RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus::
                        Accepted,
                    removed
                        ? "provider rank removed"
                        : "provider rank was not present");
        }

        case Command::SetFallback:
        {
            policy_.SetPreferredProviderFallbackEnabled(
                request.preferred_provider_fallback_enabled);

            return
                Snapshot(
                    RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus::
                        Accepted,
                    request.preferred_provider_fallback_enabled
                        ? "preferred-provider fallback enabled"
                        : "preferred-provider fallback disabled");
        }

        case Command::Query:
        {
            return
                Snapshot(
                    RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus::
                        Accepted,
                    "provider-selection policy snapshot");
        }
    }

    return
        Snapshot(
            RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus::
                UnknownCommand,
            "provider-selection policy command is unknown");
}

RuntimeServiceCapabilityProviderSelectionPolicyProtocolResult
RuntimeServiceCapabilityProviderSelectionPolicyService::
Snapshot(
    const
    RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus
        status,
    std::string diagnostic) const
{
    return
        RuntimeServiceCapabilityProviderSelectionPolicyProtocolResult{
            .status = status,
            .provider_ranks = policy_.ProviderRanks(),
            .selection_count =
                policy_.SelectionCount(),
            .ranked_selection_count =
                policy_.RankedSelectionCount(),
            .preferred_selection_count =
                policy_.PreferredSelectionCount(),
            .preferred_fallback_count =
                policy_.PreferredFallbackCount(),
            .preferred_provider_fallback_enabled =
                policy_.PreferredProviderFallbackEnabled(),
            .diagnostic = std::move(diagnostic)};
}

} // namespace sovereign
