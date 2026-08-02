#include "runtime_service_capability_trace_query.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <string>
#include "test_require.hpp"

namespace
{

sovereign::RuntimeServiceCapabilityPipelineTrace
MakeTrace(
    const std::uint64_t sequence,
    const char* capability,
    const char* operation,
    const char* service_name,
    const bool completed,
    const std::uint8_t result_status)
{
    sovereign::RuntimeServiceCapabilityPipelineTrace trace;

    trace.invocation_sequence = sequence;
    trace.capability = capability;
    trace.operation = operation;
    trace.selected_service_name = service_name;

    trace.request_policy_evaluated = true;
    trace.negotiation_attempted = true;
    trace.provider_policy_evaluated = true;
    trace.invocation_attempted = true;

    trace.completed = completed;
    trace.result_status = result_status;

    return trace;
}

} // namespace

int main()
{
    using namespace sovereign;

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        traces;

    traces.push_back(
        MakeTrace(
            1U,
            "alpha",
            "execute",
            "service-a",
            true,
            0U));

    traces.push_back(
        MakeTrace(
            2U,
            "beta",
            "inspect",
            "service-b",
            false,
            4U));

    traces.push_back(
        MakeTrace(
            3U,
            "alpha",
            "execute",
            "service-c",
            true,
            0U));

    /*
     * An empty query reports no active filters and retains
     * all execution-shaping defaults in its detached copy.
     */
    const auto unfiltered =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    REQUIRE(
        unfiltered.query_diagnostics.active_filters ==
        0U);

    const auto& unfiltered_effective =
        unfiltered.query_diagnostics.effective_query;

    REQUIRE(
        !unfiltered_effective.capability.has_value());
    REQUIRE(
        !unfiltered_effective.operation.has_value());
    REQUIRE(
        !unfiltered_effective
             .selected_service_name
             .has_value());
    REQUIRE(
        !unfiltered_effective
             .request_policy_evaluated
             .has_value());
    REQUIRE(
        !unfiltered_effective
             .negotiation_attempted
             .has_value());
    REQUIRE(
        !unfiltered_effective
             .provider_policy_evaluated
             .has_value());
    REQUIRE(
        !unfiltered_effective
             .invocation_attempted
             .has_value());
    REQUIRE(
        !unfiltered_effective.completed.has_value());
    REQUIRE(
        !unfiltered_effective.result_status.has_value());

    REQUIRE(
        unfiltered_effective.sort_order ==
        RuntimeServiceCapabilityTraceSortOrder::
            OldestFirst);

    REQUIRE(unfiltered_effective.offset == 0U);

    REQUIRE(
        unfiltered_effective.maximum_results ==
        std::numeric_limits<std::size_t>::max());

    REQUIRE(unfiltered.matched_count == 3U);
    REQUIRE(unfiltered.returned_count == 3U);

    /*
     * One populated field produces one active filter.
     */
    RuntimeServiceCapabilityTraceQuery
        capability_query;

    capability_query.capability = "alpha";

    const auto capability_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            capability_query);

    REQUIRE(
        capability_result.query_diagnostics
            .active_filters ==
        1U);

    REQUIRE(
        capability_result.query_diagnostics
            .effective_query.capability ==
        capability_query.capability);

    REQUIRE(capability_result.matched_count == 2U);

    /*
     * Every supported filter participates in the count.
     */
    RuntimeServiceCapabilityTraceQuery
        full_query;

    full_query.capability = "alpha";
    full_query.operation = "execute";
    full_query.selected_service_name =
        "service-a";

    full_query.request_policy_evaluated = true;
    full_query.negotiation_attempted = true;
    full_query.provider_policy_evaluated = true;
    full_query.invocation_attempted = true;
    full_query.completed = true;
    full_query.result_status = 0U;

    full_query.sort_order =
        RuntimeServiceCapabilityTraceSortOrder::
            NewestFirst;

    full_query.offset = 0U;
    full_query.maximum_results = 1U;

    const auto full_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            full_query);

    REQUIRE(
        full_result.query_diagnostics.active_filters ==
        9U);

    const auto& full_effective =
        full_result.query_diagnostics.effective_query;

    REQUIRE(
        full_effective.capability ==
        full_query.capability);

    REQUIRE(
        full_effective.operation ==
        full_query.operation);

    REQUIRE(
        full_effective.selected_service_name ==
        full_query.selected_service_name);

    REQUIRE(
        full_effective.request_policy_evaluated ==
        full_query.request_policy_evaluated);

    REQUIRE(
        full_effective.negotiation_attempted ==
        full_query.negotiation_attempted);

    REQUIRE(
        full_effective.provider_policy_evaluated ==
        full_query.provider_policy_evaluated);

    REQUIRE(
        full_effective.invocation_attempted ==
        full_query.invocation_attempted);

    REQUIRE(
        full_effective.completed ==
        full_query.completed);

    REQUIRE(
        full_effective.result_status ==
        full_query.result_status);

    REQUIRE(
        full_effective.sort_order ==
        RuntimeServiceCapabilityTraceSortOrder::
            NewestFirst);

    REQUIRE(full_effective.offset == 0U);
    REQUIRE(full_effective.maximum_results == 1U);

    REQUIRE(full_result.matched_count == 1U);
    REQUIRE(full_result.returned_count == 1U);
    REQUIRE(!full_result.has_more);

    /*
     * Pagination and ordering are preserved in the effective
     * query but do not increase active_filters.
     */
    RuntimeServiceCapabilityTraceQuery
        shaping_only_query;

    shaping_only_query.sort_order =
        RuntimeServiceCapabilityTraceSortOrder::
            NewestFirst;

    shaping_only_query.offset = 1U;
    shaping_only_query.maximum_results = 1U;

    const auto shaping_only_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            shaping_only_query);

    REQUIRE(
        shaping_only_result.query_diagnostics
            .active_filters ==
        0U);

    REQUIRE(
        shaping_only_result.query_diagnostics
            .effective_query.sort_order ==
        RuntimeServiceCapabilityTraceSortOrder::
            NewestFirst);

    REQUIRE(
        shaping_only_result.query_diagnostics
            .effective_query.offset ==
        1U);

    REQUIRE(
        shaping_only_result.query_diagnostics
            .effective_query.maximum_results ==
        1U);

    REQUIRE(shaping_only_result.matched_count == 3U);
    REQUIRE(shaping_only_result.returned_count == 1U);
    REQUIRE(shaping_only_result.has_more);

    /*
     * The effective query is detached from subsequent caller
     * mutation.
     */
    RuntimeServiceCapabilityTraceQuery
        detached_query;

    detached_query.capability = "alpha";
    detached_query.maximum_results = 1U;

    const auto detached_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            detached_query);

    detached_query.capability = "beta";
    detached_query.maximum_results = 0U;

    REQUIRE(
        detached_result.query_diagnostics
            .effective_query.capability
            .has_value());

    REQUIRE(
        *detached_result.query_diagnostics
             .effective_query.capability ==
        std::string{"alpha"});

    REQUIRE(
        detached_result.query_diagnostics
            .effective_query.maximum_results ==
        1U);

    REQUIRE(
        detached_result.query_diagnostics
            .active_filters ==
        1U);

    /*
     * Zero-limit metadata requests preserve query diagnostics.
     */
    RuntimeServiceCapabilityTraceQuery
        metadata_query;

    metadata_query.completed = true;
    metadata_query.maximum_results = 0U;

    const auto metadata_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    REQUIRE(
        metadata_result.query_diagnostics
            .active_filters ==
        1U);

    REQUIRE(
        metadata_result.query_diagnostics
            .effective_query.completed ==
        std::optional<bool>{true});

    REQUIRE(
        metadata_result.query_diagnostics
            .effective_query.maximum_results ==
        0U);

    REQUIRE(metadata_result.matched_count == 2U);
    REQUIRE(metadata_result.returned_count == 0U);
    REQUIRE(metadata_result.traces.empty());

    REQUIRE(
        metadata_result.execution_diagnostics
            .metadata_only);

    return 0;
}
