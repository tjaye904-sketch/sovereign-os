#include "event_bus.hpp"
#include "runtime_service_capability_invocation_policy.hpp"
#include "runtime_service_capability_invocation_protocol.hpp"
#include "runtime_service_capability_invocation_policy_protocol.hpp"
#include "runtime_service_capability_invoker.hpp"
#include "runtime_service_capability_negotiator.hpp"
#include "runtime_service_invocation.hpp"
#include "runtime_service_invoker.hpp"
#include "service_host.hpp"
#include "service_registry.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "test_require.hpp"

namespace
{

class PolicyInvocableService final
    : public sovereign::Service,
      public sovereign::RuntimeInvocableService
{
public:
    PolicyInvocableService(
        std::string name,
        std::vector<std::string_view> capabilities,
        std::string response_prefix)
        :
        name_(std::move(name)),
        capabilities_(std::move(capabilities)),
        response_prefix_(std::move(response_prefix))
    {
    }

    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return name_;
    }

    [[nodiscard]]
    std::vector<std::string_view>
    Capabilities() const override
    {
        return capabilities_;
    }

    bool Start(sovereign::EventBus&) override
    {
        return true;
    }

    bool Update(
        const sovereign::FrameContext&) override
    {
        return true;
    }

    bool Stop() override
    {
        return true;
    }

    [[nodiscard]]
    bool SupportsOperation(
        const std::string_view operation)
        const noexcept override
    {
        return
            operation == "read" ||
            operation == "delete";
    }

    bool Invoke(
        const std::string_view operation,
        const std::string_view payload,
        std::string& response) override
    {
        if (!SupportsOperation(operation))
        {
            return false;
        }

        response =
            response_prefix_ +
            std::string{payload};

        return true;
    }

private:
    std::string name_;
    std::vector<std::string_view> capabilities_;
    std::string response_prefix_;
};

} // namespace

int main()
{
    sovereign::ServiceRegistry registry;

    REQUIRE(
        registry.Register(
            std::make_unique<PolicyInvocableService>(
                "PrimaryStorage",
                std::vector<std::string_view>{
                    "storage.read",
                    "storage.delete"},
                "primary:")));

    REQUIRE(
        registry.Register(
            std::make_unique<PolicyInvocableService>(
                "ReplicaStorage",
                std::vector<std::string_view>{
                    "storage.read",
                    "storage.delete"},
                "replica:")));

    sovereign::EventBus event_bus;

    sovereign::ServiceHost host(
        event_bus,
        registry);

    REQUIRE(host.StartAll());

    sovereign::RuntimeServiceInvoker
        direct_invoker(registry);

    sovereign::RuntimeServiceCapabilityNegotiator
        negotiator(registry);

    sovereign::RuntimeServiceCapabilityInvocationPolicy
        policy;

    REQUIRE(policy.DenyCapability("storage.delete"));

    REQUIRE(
        !policy.DenyCapability("storage.delete"));

    REQUIRE(
        !policy.DenyCapability(""));

    REQUIRE(
        policy.DenyOperation(
            "storage.read",
            "delete"));

    REQUIRE(
        policy.DenyProvider(
            "storage.read",
            "ReplicaStorage"));

    REQUIRE(policy.DeniedCapabilityCount() == 1U);
    REQUIRE(policy.DeniedOperationCount() == 1U);
    REQUIRE(policy.DeniedProviderCount() == 1U);

    sovereign::RuntimeServiceCapabilityInvoker
        capability_invoker(
            negotiator,
            direct_invoker,
            policy);

    /*
     * Allowed requests preserve RT-112 behavior.
     */
    const auto allowed =
        capability_invoker.Invoke(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability = "storage.read",
                    .operation = "read",
                    .payload = "alpha"});

    REQUIRE(allowed.Completed());

    REQUIRE(
        allowed.invocation.response_payload ==
        "primary:alpha");

    /*
     * Capability denial occurs before negotiation
     * and before direct invocation.
     */
    const auto negotiation_before_capability =
        negotiator.NegotiationCount();

    const auto invocation_before_capability =
        direct_invoker.InvocationCount();

    const auto denied_capability =
        capability_invoker.Invoke(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability = "storage.delete",
                    .operation = "delete",
                    .payload = "beta"});

    REQUIRE(!denied_capability.Completed());

    REQUIRE(
        denied_capability.status ==
        sovereign::
            RuntimeServiceCapabilityInvocationStatus::
                PolicyRejected);

    REQUIRE(
        denied_capability.policy_status ==
        static_cast<std::uint8_t>(
            sovereign::
                RuntimeServiceCapabilityInvocationPolicyStatus::
                    CapabilityDenied));

    REQUIRE(
        negotiator.NegotiationCount() ==
        negotiation_before_capability);

    REQUIRE(
        direct_invoker.InvocationCount() ==
        invocation_before_capability);

    /*
     * Operation denial also occurs before
     * negotiation.
     */
    const auto negotiation_before_operation =
        negotiator.NegotiationCount();

    const auto denied_operation =
        capability_invoker.Invoke(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability = "storage.read",
                    .operation = "delete",
                    .payload = "gamma"});

    REQUIRE(!denied_operation.Completed());

    REQUIRE(
        denied_operation.policy_status ==
        static_cast<std::uint8_t>(
            sovereign::
                RuntimeServiceCapabilityInvocationPolicyStatus::
                    OperationDenied));

    REQUIRE(
        negotiator.NegotiationCount() ==
        negotiation_before_operation);

    /*
     * Provider denial occurs after deterministic
     * negotiation but before direct invocation.
     */
    const auto invocation_before_provider =
        direct_invoker.InvocationCount();

    const auto denied_provider =
        capability_invoker.Invoke(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability = "storage.read",
                    .preferred_service_name =
                        "ReplicaStorage",
                    .operation = "read",
                    .payload = "delta"});

    REQUIRE(!denied_provider.Completed());
    REQUIRE(denied_provider.negotiation.Accepted());

    REQUIRE(
        denied_provider.negotiation
            .selected_service_name ==
        "ReplicaStorage");

    REQUIRE(
        denied_provider.policy_status ==
        static_cast<std::uint8_t>(
            sovereign::
                RuntimeServiceCapabilityInvocationPolicyStatus::
                    ProviderDenied));

    REQUIRE(
        denied_provider
            .policy_provider_service_name ==
        "ReplicaStorage");

    REQUIRE(
        direct_invoker.InvocationCount() ==
        invocation_before_provider);

    /*
     * An allowed preferred provider remains
     * invocable.
     */
    const auto allowed_primary =
        capability_invoker.Invoke(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability = "storage.read",
                    .preferred_service_name =
                        "PrimaryStorage",
                    .operation = "read",
                    .payload = "epsilon"});

    REQUIRE(allowed_primary.Completed());

    REQUIRE(
        allowed_primary.invocation.response_payload ==
        "primary:epsilon");

    /*
     * Policy decisions have a stable protocol.
     */
    const auto decision =
        policy.EvaluateProvider(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability = "storage.read",
                    .operation = "read"},
            "ReplicaStorage");

    REQUIRE(!decision.Allowed());

    const auto encoded_decision =
        sovereign::
            RuntimeServiceCapabilityInvocationPolicyProtocol::
                EncodeDecision(decision);

    const auto decoded_decision =
        sovereign::
            RuntimeServiceCapabilityInvocationPolicyProtocol::
                DecodeDecision(encoded_decision);

    REQUIRE(decoded_decision.has_value());

    REQUIRE(
        decoded_decision->status ==
        sovereign::
            RuntimeServiceCapabilityInvocationPolicyStatus::
                ProviderDenied);

    REQUIRE(
        decoded_decision->provider_service_name ==
        "ReplicaStorage");

    /*
     * RT-112 result protocol now preserves policy
     * observability fields.
     */
    const auto encoded_result =
        sovereign::
            RuntimeServiceCapabilityInvocationProtocol::
                EncodeResult(denied_provider);

    const auto decoded_result =
        sovereign::
            RuntimeServiceCapabilityInvocationProtocol::
                DecodeResult(encoded_result);

    REQUIRE(decoded_result.has_value());

    REQUIRE(
        decoded_result->status ==
        sovereign::
            RuntimeServiceCapabilityInvocationStatus::
                PolicyRejected);

    REQUIRE(
        decoded_result->policy_status ==
        denied_provider.policy_status);

    REQUIRE(
        decoded_result
            ->policy_provider_service_name ==
        "ReplicaStorage");

    REQUIRE(capability_invoker.InvocationCount() == 5U);
    REQUIRE(capability_invoker.CompletedCount() == 2U);

    REQUIRE(
        capability_invoker.PolicyRejectedCount() ==
        3U);

    REQUIRE(host.StopAll());

    return 0;
}
