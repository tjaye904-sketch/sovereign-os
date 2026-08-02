#include "runtime.hpp"

#include "runtime_message_request.hpp"
#include "runtime_service_capability_invocation_policy_protocol.hpp"
#include "runtime_service_capability_invocation_policy_service.hpp"
#include "runtime_service_capability_invocation_protocol.hpp"
#include "runtime_service_capability_provider_selection_policy_service.hpp"
#include "runtime_service_endpoint_protocol.hpp"
#include "runtime_service_invocation.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include "test_require.hpp"

namespace
{

class ComposedStorageService final
    : public sovereign::Service,
      public sovereign::RuntimeInvocableService
{
public:
    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return "ComposedStorageService";
    }

    [[nodiscard]]
    std::vector<std::string_view>
    Capabilities() const override
    {
        return {
            "storage.read",
            "storage.delete"};
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
            std::string{operation} +
            ":" +
            std::string{payload};

        return true;
    }
};

sovereign::RuntimeMessageRequest
MakeEndpointRequest(
    const sovereign::RuntimeServiceEndpointTarget
        target,
    std::string target_name,
    std::string operation,
    std::string payload)
{
    return sovereign::RuntimeMessageRequest{
        .request_topic =
            std::string{
                sovereign::RuntimeServiceEndpoint::
                    RequestTopic},
        .response_topic =
            std::string{
                sovereign::RuntimeServiceEndpoint::
                    ResponseTopic},
        .payload =
            sovereign::RuntimeServiceEndpointProtocol::
                EncodeRequest(
                    sovereign::
                        RuntimeServiceEndpointRequest{
                            .target = target,
                            .target_name =
                                std::move(
                                    target_name),
                            .operation =
                                std::move(
                                    operation),
                            .payload =
                                std::move(
                                    payload)}),
        .security =
            sovereign::EventSecurity::Restricted,
        .maximum_process_cycles = 2U};
}

sovereign::RuntimeServiceEndpointResult
DecodeEndpointResult(
    const sovereign::RuntimeMessageRequestResult&
        response)
{
    REQUIRE(response.Completed());

    const auto decoded =
        sovereign::RuntimeServiceEndpointProtocol::
            DecodeResult(
                response.response_payload);

    REQUIRE(decoded.has_value());

    return *decoded;
}

} // namespace

int main()
{
    sovereign::Runtime runtime;

    /*
     * Runtime construction installs both policy
     * control services before startup.
     */
    REQUIRE(runtime.Services().Size() == 2U);

    REQUIRE(
        runtime.Services().Contains(
            sovereign::
                RuntimeServiceCapabilityProviderSelectionPolicyService::
                    ServiceName));

    REQUIRE(
        runtime.Services().Contains(
            sovereign::
                RuntimeServiceCapabilityInvocationPolicyService::
                    ServiceName));

    REQUIRE(
        runtime.RegisterService(
            std::make_unique<
                ComposedStorageService>()));

    REQUIRE(runtime.Services().Size() == 3U);

    REQUIRE(runtime.Start());

    REQUIRE(
        runtime.ServiceEndpoint().Running());

    /*
     * The runtime-owned invocation policy is
     * controlled through its registered service and
     * the runtime-owned unified endpoint.
     */
    const auto policy_request =
        sovereign::
            RuntimeServiceCapabilityInvocationPolicyProtocol::
                EncodeRequest(
                    sovereign::
                        RuntimeServiceCapabilityInvocationPolicyProtocolRequest{
                            .command =
                                sovereign::
                                    RuntimeServiceCapabilityInvocationPolicyCommand::
                                        DenyOperation,
                            .capability =
                                "storage.delete",
                            .operation =
                                "delete"});

    const auto policy_response =
        runtime.Messaging().Request(
            MakeEndpointRequest(
                sovereign::
                    RuntimeServiceEndpointTarget::
                        Direct,
                std::string{
                    sovereign::
                        RuntimeServiceCapabilityInvocationPolicyService::
                            ServiceName},
                std::string{
                    sovereign::
                        RuntimeServiceCapabilityInvocationPolicyService::
                            ApplyOperation},
                policy_request));

    const auto policy_endpoint_result =
        DecodeEndpointResult(
            policy_response);

    REQUIRE(policy_endpoint_result.Completed());

    const auto* policy_invocation =
        std::get_if<
            sovereign::
                RuntimeServiceInvocationResult>(
                    &policy_endpoint_result.result);

    REQUIRE(policy_invocation != nullptr);
    REQUIRE(policy_invocation->Completed());

    const auto policy_result =
        sovereign::
            RuntimeServiceCapabilityInvocationPolicyProtocol::
                DecodeResult(
                    policy_invocation
                        ->response_payload);

    REQUIRE(policy_result.has_value());
    REQUIRE(policy_result->Accepted());

    REQUIRE(
        policy_result
            ->denied_operation_count ==
        1U);

    /*
     * Allowed capability invocation flows through
     * the composed negotiator and invoker.
     */
    const auto allowed_response =
        runtime.Messaging().Request(
            MakeEndpointRequest(
                sovereign::
                    RuntimeServiceEndpointTarget::
                        CapabilityInvocation,
                "storage.read",
                "read",
                "alpha"));

    const auto allowed_endpoint_result =
        DecodeEndpointResult(
            allowed_response);

    REQUIRE(allowed_endpoint_result.Completed());

    const auto* allowed_invocation =
        std::get_if<
            sovereign::
                RuntimeServiceCapabilityInvocationResult>(
                    &allowed_endpoint_result.result);

    REQUIRE(allowed_invocation != nullptr);
    REQUIRE(allowed_invocation->Completed());

    REQUIRE(
        allowed_invocation
            ->invocation.response_payload ==
        "read:alpha");

    /*
     * The service-mediated policy mutation affects
     * the same invocation-policy instance wired into
     * the runtime capability invoker.
     */
    const auto denied_response =
        runtime.Messaging().Request(
            MakeEndpointRequest(
                sovereign::
                    RuntimeServiceEndpointTarget::
                        CapabilityInvocation,
                "storage.delete",
                "delete",
                "beta"));

    const auto denied_endpoint_result =
        DecodeEndpointResult(
            denied_response);

    REQUIRE(
        !denied_endpoint_result.Completed());

    const auto* denied_invocation =
        std::get_if<
            sovereign::
                RuntimeServiceCapabilityInvocationResult>(
                    &denied_endpoint_result.result);

    REQUIRE(denied_invocation != nullptr);

    REQUIRE(
        denied_invocation->status ==
        sovereign::
            RuntimeServiceCapabilityInvocationStatus::
                PolicyRejected);

    REQUIRE(
        runtime
            .CapabilityInvocationPolicy()
            .DeniedOperationCount() ==
        1U);

    /*
     * RT-120 records one immutable pipeline trace
     * for each composed capability invocation.
     */
    const auto& traces =
        runtime.CapabilityPipelineTraces();

    REQUIRE(traces.size() == 2U);

    const auto& allowed_trace =
        traces.front();

    REQUIRE(!allowed_trace.trace_id.IsNil());

    REQUIRE(
        allowed_trace.invocation_sequence ==
        allowed_invocation
            ->capability_invocation_sequence);

    REQUIRE(
        allowed_trace.capability ==
        "storage.read");

    REQUIRE(
        allowed_trace.operation ==
        "read");

    REQUIRE(
        allowed_trace.request_policy_evaluated);

    REQUIRE(
        allowed_trace.negotiation_attempted);

    REQUIRE(
        allowed_trace.candidate_count ==
        allowed_invocation
            ->negotiation.candidate_count);

    REQUIRE(
        allowed_trace.selected_service_name ==
        allowed_invocation
            ->negotiation
            .selected_service_name);

    REQUIRE(
        allowed_trace.provider_policy_evaluated);

    REQUIRE(
        allowed_trace.invocation_attempted);

    REQUIRE(allowed_trace.completed);

    REQUIRE(
        allowed_trace.ended_nanoseconds >=
        allowed_trace.started_nanoseconds);

    REQUIRE(
        allowed_trace
            .request_policy_ended_nanoseconds >=
        allowed_trace
            .request_policy_started_nanoseconds);

    REQUIRE(
        allowed_trace
            .negotiation_ended_nanoseconds >=
        allowed_trace
            .negotiation_started_nanoseconds);

    REQUIRE(
        allowed_trace
            .provider_policy_ended_nanoseconds >=
        allowed_trace
            .provider_policy_started_nanoseconds);

    REQUIRE(
        allowed_trace
            .invocation_ended_nanoseconds >=
        allowed_trace
            .invocation_started_nanoseconds);

    const auto& denied_trace =
        traces.back();

    REQUIRE(!denied_trace.trace_id.IsNil());

    REQUIRE(
        denied_trace.trace_id !=
        allowed_trace.trace_id);

    REQUIRE(
        denied_trace.capability ==
        "storage.delete");

    REQUIRE(
        denied_trace.operation ==
        "delete");

    REQUIRE(
        denied_trace.request_policy_evaluated);

    /*
     * Operation denial occurs before negotiation,
     * provider policy evaluation, and invocation.
     */
    REQUIRE(
        !denied_trace.negotiation_attempted);

    REQUIRE(
        !denied_trace.provider_policy_evaluated);

    REQUIRE(
        !denied_trace.invocation_attempted);

    REQUIRE(!denied_trace.completed);

    REQUIRE(
        denied_trace.result_status ==
        static_cast<std::uint8_t>(
            sovereign::
                RuntimeServiceCapabilityInvocationStatus::
                    PolicyRejected));

    REQUIRE(
        runtime.LastCapabilityPipelineTrace()
            .trace_id ==
        denied_trace.trace_id);

    REQUIRE(
        runtime.CapabilityPipelineTraceCount() ==
        2U);

    runtime.ClearCapabilityPipelineTraces();

    REQUIRE(
        runtime.CapabilityPipelineTraces()
            .empty());

    REQUIRE(
        runtime
            .LastCapabilityPipelineTrace()
            .trace_id.IsNil());

    REQUIRE(runtime.Stop());

    REQUIRE(
        !runtime.ServiceEndpoint().Running());

    return 0;
}
