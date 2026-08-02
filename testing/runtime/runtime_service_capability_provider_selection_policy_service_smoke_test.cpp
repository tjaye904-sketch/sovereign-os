#include "event_bus.hpp"
#include "runtime_service_capability_provider_selection_policy_protocol.hpp"
#include "runtime_service_capability_provider_selection_policy_service.hpp"
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
        RuntimeServiceCapabilityProviderSelectionPolicyCommand;

using Protocol =
    sovereign::
        RuntimeServiceCapabilityProviderSelectionPolicyProtocol;

using ProtocolRequest =
    sovereign::
        RuntimeServiceCapabilityProviderSelectionPolicyProtocolRequest;

using ProtocolStatus =
    sovereign::
        RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus;

using Policy =
    sovereign::
        RuntimeServiceCapabilityProviderSelectionPolicy;

using PolicyService =
    sovereign::
        RuntimeServiceCapabilityProviderSelectionPolicyService;

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
RuntimeServiceCapabilityProviderSelectionPolicyProtocolResult
DecodeCompleted(
    const sovereign::RuntimeServiceInvocationResult&
        invocation)
{
    REQUIRE(invocation.Completed());

    const auto decoded =
        Protocol::DecodeResult(
            invocation.response_payload);

    REQUIRE(decoded);

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
     * SET_RANK.
     */
    const auto set_rank =
        DecodeCompleted(
            Invoke(
                invoker,
                ProtocolRequest{
                    .command = Command::SetRank,
                    .capability =
                        "analysis.generate",
                    .service_name =
                        "PrimaryAnalysisService",
                    .rank = 17U}));

    REQUIRE(set_rank.Accepted());
    REQUIRE(set_rank.provider_ranks.size() == 1U);
    REQUIRE(
        set_rank.provider_ranks[0].capability ==
        "analysis.generate");
    REQUIRE(
        set_rank.provider_ranks[0].service_name ==
        "PrimaryAnalysisService");
    REQUIRE(
        set_rank.provider_ranks[0].rank ==
        17U);

    /*
     * A second rank verifies deterministic snapshot order.
     */
    const auto second_rank =
        DecodeCompleted(
            Invoke(
                invoker,
                ProtocolRequest{
                    .command = Command::SetRank,
                    .capability =
                        "analysis.generate",
                    .service_name =
                        "SecondaryAnalysisService",
                    .rank = 23U}));

    REQUIRE(second_rank.Accepted());
    REQUIRE(second_rank.provider_ranks.size() == 2U);
    REQUIRE(
        second_rank.provider_ranks[0].service_name ==
        "PrimaryAnalysisService");
    REQUIRE(
        second_rank.provider_ranks[1].service_name ==
        "SecondaryAnalysisService");

    /*
     * SET_FALLBACK.
     */
    const auto fallback =
        DecodeCompleted(
            Invoke(
                invoker,
                ProtocolRequest{
                    .command = Command::SetFallback,
                    .preferred_provider_fallback_enabled =
                        true}));

    REQUIRE(fallback.Accepted());
    REQUIRE(
        fallback.
            preferred_provider_fallback_enabled);

    /*
     * QUERY preserves policy counters and state.
     */
    const auto query =
        DecodeCompleted(
            Invoke(
                invoker,
                ProtocolRequest{
                    .command = Command::Query}));

    REQUIRE(query.Accepted());
    REQUIRE(query.provider_ranks.size() == 2U);
    REQUIRE(
        query.preferred_provider_fallback_enabled);
    REQUIRE(query.selection_count == 0U);
    REQUIRE(query.ranked_selection_count == 0U);
    REQUIRE(query.preferred_selection_count == 0U);
    REQUIRE(query.preferred_fallback_count == 0U);

    /*
     * REMOVE_RANK.
     */
    const auto remove =
        DecodeCompleted(
            Invoke(
                invoker,
                ProtocolRequest{
                    .command = Command::RemoveRank,
                    .capability =
                        "analysis.generate",
                    .service_name =
                        "PrimaryAnalysisService"}));

    REQUIRE(remove.Accepted());
    REQUIRE(remove.provider_ranks.size() == 1U);
    REQUIRE(
        remove.provider_ranks[0].service_name ==
        "SecondaryAnalysisService");

    /*
     * Removal is idempotently accepted.
     */
    const auto remove_missing =
        DecodeCompleted(
            Invoke(
                invoker,
                ProtocolRequest{
                    .command = Command::RemoveRank,
                    .capability =
                        "analysis.generate",
                    .service_name =
                        "PrimaryAnalysisService"}));

    REQUIRE(remove_missing.Accepted());
    REQUIRE(remove_missing.provider_ranks.size() == 1U);

    /*
     * Malformed wire input returns an encoded protocol
     * result rather than failing the service invocation.
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
                    "not-a-valid-rt-115-request"});

    REQUIRE(malformed.Completed());

    const auto malformed_result =
        Protocol::DecodeResult(
            malformed.response_payload);

    REQUIRE(malformed_result);
    REQUIRE(
        malformed_result->status ==
        ProtocolStatus::InvalidPayload);

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
