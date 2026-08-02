#include "event_bus.hpp"
#include "runtime_service_capability_invocation_policy_protocol.hpp"
#include "runtime_service_capability_invocation_policy_service.hpp"
#include "runtime_service_invoker.hpp"
#include "service_host.hpp"
#include "service_registry.hpp"

#include <memory>
#include <string>
#include "test_require.hpp"

namespace
{

using Command =
    sovereign::
        RuntimeServiceCapabilityInvocationPolicyCommand;

using Protocol =
    sovereign::
        RuntimeServiceCapabilityInvocationPolicyProtocol;

using ProtocolRequest =
    sovereign::
        RuntimeServiceCapabilityInvocationPolicyProtocolRequest;

using ProtocolStatus =
    sovereign::
        RuntimeServiceCapabilityInvocationPolicyProtocolStatus;

using Policy =
    sovereign::
        RuntimeServiceCapabilityInvocationPolicy;

using PolicyService =
    sovereign::
        RuntimeServiceCapabilityInvocationPolicyService;

sovereign::RuntimeServiceInvocationResult
Invoke(
    sovereign::RuntimeServiceInvoker& invoker,
    const ProtocolRequest& request)
{
    return
        invoker.Invoke(
            sovereign::RuntimeServiceInvocationRequest{
                .service_name =
                    std::string{
                        PolicyService::ServiceName},
                .operation =
                    std::string{
                        PolicyService::ApplyOperation},
                .payload =
                    Protocol::EncodeRequest(request)});
}

sovereign::
RuntimeServiceCapabilityInvocationPolicyProtocolResult
DecodeCompleted(
    const sovereign::RuntimeServiceInvocationResult&
        invocation)
{
    REQUIRE(invocation.Completed());

    const auto decoded =
        Protocol::DecodeResult(
            invocation.response_payload);

    REQUIRE(decoded.has_value());

    return *decoded;
}

} // namespace

int main()
{
    sovereign::EventBus event_bus;
    sovereign::ServiceRegistry registry;

    sovereign::ServiceHost host(
        event_bus,
        registry);

    Policy policy{};

    REQUIRE(
        registry.Register(
            std::make_unique<PolicyService>(
                policy)));

    sovereign::RuntimeServiceInvoker
        invoker(registry);

    /*
     * Registered but not running.
     */
    const auto stopped =
        Invoke(
            invoker,
            ProtocolRequest{
                .command = Command::Query});

    REQUIRE(!stopped.Completed());

    REQUIRE(
        stopped.status ==
        sovereign::RuntimeServiceInvocationStatus::
            ServiceNotRunning);

    REQUIRE(host.StartAll());

    /*
     * Service identity and operation contract.
     */
    const sovereign::Service* registered =
        registry.Find(
            PolicyService::ServiceName);

    REQUIRE(registered != nullptr);

    REQUIRE(
        registered->Name() ==
        PolicyService::ServiceName);

    const auto unsupported =
        invoker.Invoke(
            sovereign::RuntimeServiceInvocationRequest{
                .service_name =
                    std::string{
                        PolicyService::ServiceName},
                .operation = "unsupported",
                .payload = {}});

    REQUIRE(
        unsupported.status ==
        sovereign::RuntimeServiceInvocationStatus::
            OperationRejected);

    /*
     * DENY_CAPABILITY.
     */
    const auto deny_capability =
        DecodeCompleted(
            Invoke(
                invoker,
                ProtocolRequest{
                    .command =
                        Command::DenyCapability,
                    .capability =
                        "analysis.generate"}));

    REQUIRE(deny_capability.Accepted());

    REQUIRE(
        deny_capability.denied_capability_count ==
        1U);

    REQUIRE(
        deny_capability.denied_operation_count ==
        0U);

    REQUIRE(
        deny_capability.denied_provider_count ==
        0U);

    /*
     * Duplicate denial is idempotently accepted.
     */
    const auto duplicate_capability =
        DecodeCompleted(
            Invoke(
                invoker,
                ProtocolRequest{
                    .command =
                        Command::DenyCapability,
                    .capability =
                        "analysis.generate"}));

    REQUIRE(duplicate_capability.Accepted());

    REQUIRE(
        duplicate_capability
            .denied_capability_count ==
        1U);

    /*
     * DENY_OPERATION.
     */
    const auto deny_operation =
        DecodeCompleted(
            Invoke(
                invoker,
                ProtocolRequest{
                    .command =
                        Command::DenyOperation,
                    .capability =
                        "storage.read",
                    .operation =
                        "read-private"}));

    REQUIRE(deny_operation.Accepted());

    REQUIRE(
        deny_operation.denied_capability_count ==
        1U);

    REQUIRE(
        deny_operation.denied_operation_count ==
        1U);

    /*
     * DENY_PROVIDER.
     */
    const auto deny_provider =
        DecodeCompleted(
            Invoke(
                invoker,
                ProtocolRequest{
                    .command =
                        Command::DenyProvider,
                    .capability =
                        "storage.read",
                    .provider_service_name =
                        "RestrictedStorageService"}));

    REQUIRE(deny_provider.Accepted());

    REQUIRE(
        deny_provider.denied_provider_count ==
        1U);

    /*
     * QUERY preserves accumulated policy state.
     */
    const auto query =
        DecodeCompleted(
            Invoke(
                invoker,
                ProtocolRequest{
                    .command = Command::Query}));

    REQUIRE(query.Accepted());

    REQUIRE(
        query.denied_capability_count ==
        1U);

    REQUIRE(
        query.denied_operation_count ==
        1U);

    REQUIRE(
        query.denied_provider_count ==
        1U);

    /*
     * Service mutations affect the underlying
     * invocation policy.
     */
    const auto capability_decision =
        policy.EvaluateRequest(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability =
                        "analysis.generate",
                    .operation =
                        "summarize"});

    REQUIRE(
        capability_decision.status ==
        sovereign::
            RuntimeServiceCapabilityInvocationPolicyStatus::
                CapabilityDenied);

    const auto operation_decision =
        policy.EvaluateRequest(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability =
                        "storage.read",
                    .operation =
                        "read-private"});

    REQUIRE(
        operation_decision.status ==
        sovereign::
            RuntimeServiceCapabilityInvocationPolicyStatus::
                OperationDenied);

    const auto provider_decision =
        policy.EvaluateProvider(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability =
                        "storage.read",
                    .operation =
                        "read-public"},
            "RestrictedStorageService");

    REQUIRE(
        provider_decision.status ==
        sovereign::
            RuntimeServiceCapabilityInvocationPolicyStatus::
                ProviderDenied);

    /*
     * Malformed input returns an encoded protocol
     * result instead of failing service invocation.
     */
    const auto malformed =
        invoker.Invoke(
            sovereign::RuntimeServiceInvocationRequest{
                .service_name =
                    std::string{
                        PolicyService::ServiceName},
                .operation =
                    std::string{
                        PolicyService::ApplyOperation},
                .payload =
                    "not-a-valid-rt-117-request"});

    REQUIRE(malformed.Completed());

    const auto malformed_result =
        Protocol::DecodeResult(
            malformed.response_payload);

    REQUIRE(malformed_result.has_value());

    REQUIRE(
        malformed_result->status ==
        ProtocolStatus::InvalidPayload);

    REQUIRE(
        malformed_result
            ->denied_capability_count ==
        1U);

    REQUIRE(
        malformed_result
            ->denied_operation_count ==
        1U);

    REQUIRE(
        malformed_result
            ->denied_provider_count ==
        1U);

    /*
     * Service shutdown restores normal invocation
     * lifecycle enforcement.
     */
    REQUIRE(host.StopAll());

    const auto stopped_again =
        Invoke(
            invoker,
            ProtocolRequest{
                .command = Command::Query});

    REQUIRE(
        stopped_again.status ==
        sovereign::RuntimeServiceInvocationStatus::
            ServiceNotRunning);

    return 0;
}
