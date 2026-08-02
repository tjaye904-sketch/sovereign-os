#include "runtime_service_capability_invocation_policy_service.hpp"

#include <utility>

namespace sovereign
{

RuntimeServiceCapabilityInvocationPolicyService::
RuntimeServiceCapabilityInvocationPolicyService(
    RuntimeServiceCapabilityInvocationPolicy&
        policy) noexcept
    : policy_(policy)
{
}

std::string_view
RuntimeServiceCapabilityInvocationPolicyService::
Name() const noexcept
{
    return ServiceName;
}

bool
RuntimeServiceCapabilityInvocationPolicyService::
Start(EventBus&)
{
    return true;
}

bool
RuntimeServiceCapabilityInvocationPolicyService::
Update(const FrameContext&)
{
    return true;
}

bool
RuntimeServiceCapabilityInvocationPolicyService::
Stop()
{
    return true;
}

bool
RuntimeServiceCapabilityInvocationPolicyService::
SupportsOperation(
    const std::string_view operation)
    const noexcept
{
    return operation == ApplyOperation;
}

bool
RuntimeServiceCapabilityInvocationPolicyService::
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
        RuntimeServiceCapabilityInvocationPolicyProtocol::
            DecodeRequest(request_payload);

    RuntimeServiceCapabilityInvocationPolicyProtocolResult
        result{};

    if (!request)
    {
        result =
            Snapshot(
                RuntimeServiceCapabilityInvocationPolicyProtocolStatus::
                    InvalidPayload,
                "capability invocation policy request payload is invalid");
    }
    else
    {
        result = Execute(*request);
    }

    response_payload =
        RuntimeServiceCapabilityInvocationPolicyProtocol::
            EncodeResult(result);

    return true;
}

RuntimeServiceCapabilityInvocationPolicyProtocolResult
RuntimeServiceCapabilityInvocationPolicyService::
Execute(
    const
    RuntimeServiceCapabilityInvocationPolicyProtocolRequest&
        request)
{
    using Command =
        RuntimeServiceCapabilityInvocationPolicyCommand;

    switch (request.command)
    {
        case Command::DenyCapability:
        {
            const bool changed =
                policy_.DenyCapability(
                    request.capability);

            return
                Snapshot(
                    RuntimeServiceCapabilityInvocationPolicyProtocolStatus::
                        Accepted,
                    changed
                        ? "capability denial stored"
                        : "capability denial unchanged");
        }

        case Command::DenyOperation:
        {
            const bool changed =
                policy_.DenyOperation(
                    request.capability,
                    request.operation);

            return
                Snapshot(
                    RuntimeServiceCapabilityInvocationPolicyProtocolStatus::
                        Accepted,
                    changed
                        ? "operation denial stored"
                        : "operation denial unchanged");
        }

        case Command::DenyProvider:
        {
            const bool changed =
                policy_.DenyProvider(
                    request.capability,
                    request.provider_service_name);

            return
                Snapshot(
                    RuntimeServiceCapabilityInvocationPolicyProtocolStatus::
                        Accepted,
                    changed
                        ? "provider denial stored"
                        : "provider denial unchanged");
        }

        case Command::Query:
        {
            return
                Snapshot(
                    RuntimeServiceCapabilityInvocationPolicyProtocolStatus::
                        Accepted,
                    "capability invocation policy snapshot");
        }
    }

    return
        Snapshot(
            RuntimeServiceCapabilityInvocationPolicyProtocolStatus::
                UnknownCommand,
            "capability invocation policy command is unknown");
}

RuntimeServiceCapabilityInvocationPolicyProtocolResult
RuntimeServiceCapabilityInvocationPolicyService::
Snapshot(
    const
    RuntimeServiceCapabilityInvocationPolicyProtocolStatus
        status,
    std::string diagnostic) const
{
    return
        RuntimeServiceCapabilityInvocationPolicyProtocolResult{
            .status = status,
            .denied_capability_count =
                policy_.DeniedCapabilityCount(),
            .denied_operation_count =
                policy_.DeniedOperationCount(),
            .denied_provider_count =
                policy_.DeniedProviderCount(),
            .diagnostic = std::move(diagnostic)};
}

} // namespace sovereign
