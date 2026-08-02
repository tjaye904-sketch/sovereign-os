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
     *
     * RT-121 bounds the retained history while
     * preserving the RT-120 trace contract.
     */
    REQUIRE(
        runtime.CapabilityPipelineTraceCapacity() ==
        sovereign::
            RuntimeServiceCapabilityInvoker::
                DefaultTraceCapacity);

    REQUIRE(
        runtime
            .DiscardedCapabilityPipelineTraceCount() ==
        0U);

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

    /*
     * RT-122 empty queries return all retained
     * traces in deterministic oldest-first order.
     */
    const auto all_oldest_first =
        runtime.QueryCapabilityPipelineTraces(
            sovereign::
                RuntimeServiceCapabilityTraceQuery{});

    REQUIRE(all_oldest_first.matched_count == 2U);
    REQUIRE(all_oldest_first.returned_count == 2U);
    REQUIRE(!all_oldest_first.truncated);

    REQUIRE(
        all_oldest_first.traces.front().trace_id ==
        allowed_trace.trace_id);

    REQUIRE(
        all_oldest_first.traces.back().trace_id ==
        denied_trace.trace_id);

    /*
     * Newest-first traversal requires no
     * intermediate sorting.
     */
    sovereign::
        RuntimeServiceCapabilityTraceQuery
            newest_query;

    newest_query.sort_order =
        sovereign::
            RuntimeServiceCapabilityTraceSortOrder::
                NewestFirst;

    const auto all_newest_first =
        runtime.QueryCapabilityPipelineTraces(
            newest_query);

    REQUIRE(all_newest_first.matched_count == 2U);
    REQUIRE(all_newest_first.returned_count == 2U);

    REQUIRE(
        all_newest_first.traces.front().trace_id ==
        denied_trace.trace_id);

    REQUIRE(
        all_newest_first.traces.back().trace_id ==
        allowed_trace.trace_id);

    /*
     * Capability and operation filters compose.
     */
    sovereign::
        RuntimeServiceCapabilityTraceQuery
            allowed_query;

    allowed_query.capability =
        std::string{"storage.read"};

    allowed_query.operation =
        std::string{"read"};

    const auto allowed_matches =
        runtime.QueryCapabilityPipelineTraces(
            allowed_query);

    REQUIRE(allowed_matches.matched_count == 1U);
    REQUIRE(allowed_matches.returned_count == 1U);
    REQUIRE(!allowed_matches.truncated);

    REQUIRE(
        allowed_matches.traces.front().trace_id ==
        allowed_trace.trace_id);

    REQUIRE(
        allowed_matches
            .traces
            .front()
            .selected_service_name ==
        "ComposedStorageService");

    /*
     * Provider and completion filters identify
     * completed provider-backed invocations.
     */
    sovereign::
        RuntimeServiceCapabilityTraceQuery
            provider_query;

    provider_query.selected_service_name =
        std::string{"ComposedStorageService"};

    provider_query.completed = true;
    provider_query.invocation_attempted = true;

    const auto provider_matches =
        runtime.QueryCapabilityPipelineTraces(
            provider_query);

    REQUIRE(provider_matches.matched_count == 1U);
    REQUIRE(provider_matches.returned_count == 1U);

    REQUIRE(
        provider_matches.traces.front().trace_id ==
        allowed_trace.trace_id);

    /*
     * Policy rejection is queryable through the
     * immutable diagnostic status field.
     */
    sovereign::
        RuntimeServiceCapabilityTraceQuery
            rejected_query;

    rejected_query.completed = false;
    rejected_query.negotiation_attempted = false;
    rejected_query.invocation_attempted = false;

    rejected_query.result_status =
        static_cast<std::uint8_t>(
            sovereign::
                RuntimeServiceCapabilityInvocationStatus::
                    PolicyRejected);

    const auto rejected_matches =
        runtime.QueryCapabilityPipelineTraces(
            rejected_query);

    REQUIRE(rejected_matches.matched_count == 1U);
    REQUIRE(rejected_matches.returned_count == 1U);

    REQUIRE(
        rejected_matches.traces.front().trace_id ==
        denied_trace.trace_id);

    /*
     * Result limits preserve the total match count
     * and report deterministic truncation.
     */
    sovereign::
        RuntimeServiceCapabilityTraceQuery
            limited_query;

    limited_query.maximum_results = 1U;

    const auto limited_matches =
        runtime.QueryCapabilityPipelineTraces(
            limited_query);

    REQUIRE(limited_matches.matched_count == 2U);
    REQUIRE(limited_matches.returned_count == 1U);
    REQUIRE(limited_matches.truncated);

    REQUIRE(
        limited_matches.traces.front().trace_id ==
        allowed_trace.trace_id);

    limited_query.maximum_results = 0U;

    const auto zero_limit_matches =
        runtime.QueryCapabilityPipelineTraces(
            limited_query);

    REQUIRE(zero_limit_matches.matched_count == 2U);
    REQUIRE(zero_limit_matches.returned_count == 0U);
    REQUIRE(zero_limit_matches.traces.empty());
    REQUIRE(zero_limit_matches.truncated);

    /*
     * UUID lookup returns an immutable trace copy
     * and reports misses without mutation.
     */
    const auto found_allowed =
        runtime.FindCapabilityPipelineTrace(
            allowed_trace.trace_id);

    REQUIRE(found_allowed.has_value());

    REQUIRE(
        found_allowed->trace_id ==
        allowed_trace.trace_id);

    REQUIRE(
        found_allowed->capability ==
        "storage.read");

    REQUIRE(
        !runtime
             .FindCapabilityPipelineTrace(
                 sovereign::Uuid{})
             .has_value());

    REQUIRE(
        runtime.CapabilityPipelineTraceCount() ==
        2U);

    REQUIRE(
        runtime
            .DiscardedCapabilityPipelineTraceCount() ==
        0U);

    /*
     * Reducing capacity immediately evicts the
     * oldest retained trace.
     */
    runtime.SetCapabilityPipelineTraceCapacity(1U);

    REQUIRE(
        runtime.CapabilityPipelineTraceCapacity() ==
        1U);

    REQUIRE(
        runtime.CapabilityPipelineTraceCount() ==
        1U);

    REQUIRE(
        runtime.CapabilityPipelineTraces()
            .front()
            .trace_id ==
        denied_trace.trace_id);

    REQUIRE(
        runtime.LastCapabilityPipelineTrace()
            .trace_id ==
        denied_trace.trace_id);

    REQUIRE(
        runtime
            .DiscardedCapabilityPipelineTraceCount() ==
        1U);

    /*
     * Zero capacity disables retention and discards
     * all currently retained traces.
     */
    runtime.SetCapabilityPipelineTraceCapacity(0U);

    REQUIRE(
        runtime.CapabilityPipelineTraceCapacity() ==
        0U);

    REQUIRE(
        runtime.CapabilityPipelineTraces()
            .empty());

    REQUIRE(
        runtime
            .LastCapabilityPipelineTrace()
            .trace_id.IsNil());

    REQUIRE(
        runtime
            .DiscardedCapabilityPipelineTraceCount() ==
        2U);

    /*
     * Invocation behavior remains active while
     * diagnostics retention is disabled.
     */
    const auto unretained_response =
        runtime.Messaging().Request(
            MakeEndpointRequest(
                sovereign::
                    RuntimeServiceEndpointTarget::
                        CapabilityInvocation,
                "storage.read",
                "read",
                "gamma"));

    const auto unretained_endpoint_result =
        DecodeEndpointResult(
            unretained_response);

    REQUIRE(
        unretained_endpoint_result.Completed());

    const auto* unretained_invocation =
        std::get_if<
            sovereign::
                RuntimeServiceCapabilityInvocationResult>(
                    &unretained_endpoint_result.result);

    REQUIRE(unretained_invocation != nullptr);
    REQUIRE(unretained_invocation->Completed());

    REQUIRE(
        unretained_invocation
            ->invocation.response_payload ==
        "read:gamma");

    REQUIRE(
        runtime.CapabilityPipelineTraceCount() ==
        0U);

    REQUIRE(
        runtime
            .LastCapabilityPipelineTrace()
            .trace_id.IsNil());

    REQUIRE(
        runtime
            .DiscardedCapabilityPipelineTraceCount() ==
        3U);

    /*
     * Querying disabled retention returns an empty
     * immutable result and preserves discard
     * accounting.
     */
    const auto disabled_query_result =
        runtime.QueryCapabilityPipelineTraces(
            sovereign::
                RuntimeServiceCapabilityTraceQuery{});

    REQUIRE(
        disabled_query_result.matched_count ==
        0U);

    REQUIRE(
        disabled_query_result.returned_count ==
        0U);

    REQUIRE(
        disabled_query_result.traces.empty());

    REQUIRE(!disabled_query_result.truncated);

    REQUIRE(
        !runtime
             .FindCapabilityPipelineTrace(
                 denied_trace.trace_id)
             .has_value());

    REQUIRE(
        runtime
            .DiscardedCapabilityPipelineTraceCount() ==
        3U);

    /*
     * Re-enabling retention affects subsequent
     * invocations without resetting discard history.
     */
    runtime.SetCapabilityPipelineTraceCapacity(2U);

    const auto retained_response =
        runtime.Messaging().Request(
            MakeEndpointRequest(
                sovereign::
                    RuntimeServiceEndpointTarget::
                        CapabilityInvocation,
                "storage.read",
                "read",
                "delta"));

    const auto retained_endpoint_result =
        DecodeEndpointResult(
            retained_response);

    REQUIRE(retained_endpoint_result.Completed());

    REQUIRE(
        runtime.CapabilityPipelineTraceCount() ==
        1U);

    REQUIRE(
        runtime
            .CapabilityPipelineTraces()
            .back()
            .capability ==
        "storage.read");

    REQUIRE(
        runtime
            .CapabilityPipelineTraces()
            .back()
            .operation ==
        "read");

    REQUIRE(
        !runtime
             .LastCapabilityPipelineTrace()
             .trace_id
             .IsNil());

    REQUIRE(
        runtime
            .DiscardedCapabilityPipelineTraceCount() ==
        3U);

    runtime.ClearCapabilityPipelineTraces();

    REQUIRE(
        runtime.CapabilityPipelineTraces()
            .empty());

    REQUIRE(
        runtime
            .LastCapabilityPipelineTrace()
            .trace_id.IsNil());

    /*
     * Explicit clearing is not a retention discard
     * and does not reset the cumulative counter.
     */
    REQUIRE(
        runtime
            .DiscardedCapabilityPipelineTraceCount() ==
        3U);

    REQUIRE(
        runtime.CapabilityPipelineTraceCapacity() ==
        2U);

    REQUIRE(runtime.Stop());

    REQUIRE(
        !runtime.ServiceEndpoint().Running());

    return 0;
}
