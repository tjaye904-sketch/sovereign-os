#include "runtime_service_capability_trace_query.hpp"

#include <cstdint>
#include <deque>
#include "test_require.hpp"

namespace
{

sovereign::RuntimeServiceCapabilityPipelineTrace
MakeTrace(
    const std::uint64_t sequence,
    const char* capability,
    const bool completed,
    const bool request_policy_evaluated,
    const bool negotiation_attempted,
    const bool provider_policy_evaluated,
    const bool invocation_attempted,
    const std::uint64_t duration)
{
    sovereign::RuntimeServiceCapabilityPipelineTrace trace;

    trace.invocation_sequence = sequence;
    trace.capability = capability;
    trace.operation = "execute";
    trace.selected_service_name = "provider";

    trace.completed = completed;

    trace.request_policy_evaluated =
        request_policy_evaluated;

    trace.negotiation_attempted =
        negotiation_attempted;

    trace.provider_policy_evaluated =
        provider_policy_evaluated;

    trace.invocation_attempted =
        invocation_attempted;

    trace.total_duration_nanoseconds = duration;

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
            true,
            true,
            true,
            true,
            true,
            10U));

    traces.push_back(
        MakeTrace(
            2U,
            "beta",
            false,
            true,
            false,
            false,
            false,
            30U));

    traces.push_back(
        MakeTrace(
            3U,
            "alpha",
            false,
            false,
            true,
            false,
            true,
            20U));

    traces.push_back(
        MakeTrace(
            4U,
            "alpha",
            true,
            true,
            true,
            false,
            true,
            40U));

    const auto all =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    REQUIRE(all.matched_count == 4U);
    REQUIRE(all.returned_count == 4U);

    REQUIRE(all.summary.completed_traces == 2U);
    REQUIRE(all.summary.incomplete_traces == 2U);

    REQUIRE(
        all.summary
            .request_policy_evaluated_traces ==
        3U);

    REQUIRE(
        all.summary.negotiation_attempted_traces ==
        3U);

    REQUIRE(
        all.summary
            .provider_policy_evaluated_traces ==
        1U);

    REQUIRE(
        all.summary.invocation_attempted_traces ==
        3U);

    REQUIRE(
        all.summary.total_duration_nanoseconds ==
        100U);

    REQUIRE(
        all.summary.minimum_duration_nanoseconds ==
        10U);

    REQUIRE(
        all.summary.maximum_duration_nanoseconds ==
        40U);

    RuntimeServiceCapabilityTraceQuery alpha_query;
    alpha_query.capability = "alpha";

    const auto alpha =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            alpha_query);

    REQUIRE(alpha.matched_count == 3U);
    REQUIRE(alpha.summary.completed_traces == 2U);
    REQUIRE(alpha.summary.incomplete_traces == 1U);

    REQUIRE(
        alpha.summary
            .request_policy_evaluated_traces ==
        2U);

    REQUIRE(
        alpha.summary.negotiation_attempted_traces ==
        3U);

    REQUIRE(
        alpha.summary
            .provider_policy_evaluated_traces ==
        1U);

    REQUIRE(
        alpha.summary.invocation_attempted_traces ==
        3U);

    REQUIRE(
        alpha.summary.total_duration_nanoseconds ==
        70U);

    REQUIRE(
        alpha.summary.minimum_duration_nanoseconds ==
        10U);

    REQUIRE(
        alpha.summary.maximum_duration_nanoseconds ==
        40U);

    RuntimeServiceCapabilityTraceQuery paged_query;

    paged_query.offset = 1U;
    paged_query.maximum_results = 1U;

    const auto paged =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            paged_query);

    REQUIRE(paged.matched_count == 4U);
    REQUIRE(paged.returned_count == 1U);
    REQUIRE(paged.has_more);

    REQUIRE(paged.summary.completed_traces == 2U);
    REQUIRE(paged.summary.incomplete_traces == 2U);

    REQUIRE(
        paged.summary.total_duration_nanoseconds ==
        100U);

    REQUIRE(
        paged.summary.minimum_duration_nanoseconds ==
        10U);

    REQUIRE(
        paged.summary.maximum_duration_nanoseconds ==
        40U);

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.maximum_results = 0U;

    const auto metadata =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    REQUIRE(metadata.matched_count == 4U);
    REQUIRE(metadata.returned_count == 0U);
    REQUIRE(metadata.traces.empty());

    REQUIRE(
        metadata.execution_diagnostics.metadata_only);

    REQUIRE(metadata.summary.completed_traces == 2U);
    REQUIRE(metadata.summary.incomplete_traces == 2U);

    REQUIRE(
        metadata.summary.total_duration_nanoseconds ==
        100U);

    RuntimeServiceCapabilityTraceQuery newest_query;

    newest_query.sort_order =
        RuntimeServiceCapabilityTraceSortOrder::
            NewestFirst;

    newest_query.maximum_results = 1U;

    const auto newest =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            newest_query);

    REQUIRE(newest.returned_count == 1U);

    REQUIRE(
        newest.traces.front().invocation_sequence ==
        4U);

    REQUIRE(newest.summary.completed_traces == 2U);
    REQUIRE(newest.summary.incomplete_traces == 2U);

    REQUIRE(
        newest.summary.total_duration_nanoseconds ==
        100U);

    RuntimeServiceCapabilityTraceQuery no_match_query;
    no_match_query.capability = "missing";

    const auto no_matches =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            no_match_query);

    REQUIRE(no_matches.matched_count == 0U);
    REQUIRE(no_matches.returned_count == 0U);

    REQUIRE(no_matches.summary.completed_traces == 0U);
    REQUIRE(no_matches.summary.incomplete_traces == 0U);

    REQUIRE(
        no_matches.summary
            .request_policy_evaluated_traces ==
        0U);

    REQUIRE(
        no_matches.summary
            .negotiation_attempted_traces ==
        0U);

    REQUIRE(
        no_matches.summary
            .provider_policy_evaluated_traces ==
        0U);

    REQUIRE(
        no_matches.summary
            .invocation_attempted_traces ==
        0U);

    REQUIRE(
        no_matches.summary
            .total_duration_nanoseconds ==
        0U);

    REQUIRE(
        no_matches.summary
            .minimum_duration_nanoseconds ==
        0U);

    REQUIRE(
        no_matches.summary
            .maximum_duration_nanoseconds ==
        0U);

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        zero_duration_traces;

    zero_duration_traces.push_back(
        MakeTrace(
            1U,
            "zero",
            true,
            false,
            false,
            false,
            false,
            0U));

    const auto zero_duration =
        QueryRuntimeServiceCapabilityTraces(
            zero_duration_traces,
            RuntimeServiceCapabilityTraceQuery{});

    REQUIRE(zero_duration.matched_count == 1U);

    REQUIRE(
        zero_duration.summary
            .total_duration_nanoseconds ==
        0U);

    REQUIRE(
        zero_duration.summary
            .minimum_duration_nanoseconds ==
        0U);

    REQUIRE(
        zero_duration.summary
            .maximum_duration_nanoseconds ==
        0U);

    return 0;
}
