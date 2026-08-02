#include "runtime_service_capability_invoker.hpp"
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
    const sovereign::
        RuntimeServiceCapabilityInvocationStatus status,
    const bool completed,
    const std::uint64_t duration)
{
    sovereign::RuntimeServiceCapabilityPipelineTrace trace;

    trace.invocation_sequence = sequence;
    trace.capability = capability;
    trace.operation = "execute";
    trace.selected_service_name = "provider";

    trace.result_status =
        static_cast<std::uint8_t>(status);

    trace.completed = completed;
    trace.total_duration_nanoseconds = duration;

    return trace;
}

std::size_t Count(
    const sovereign::
        RuntimeServiceCapabilityTraceQuerySummary&
            summary,
    const sovereign::
        RuntimeServiceCapabilityInvocationStatus status)
{
    return summary.ResultStatusCount(
        static_cast<std::uint8_t>(status));
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
            RuntimeServiceCapabilityInvocationStatus::
                Completed,
            true,
            10U));

    traces.push_back(
        MakeTrace(
            2U,
            "beta",
            RuntimeServiceCapabilityInvocationStatus::
                NegotiationRejected,
            false,
            20U));

    traces.push_back(
        MakeTrace(
            3U,
            "alpha",
            RuntimeServiceCapabilityInvocationStatus::
                PolicyRejected,
            false,
            30U));

    traces.push_back(
        MakeTrace(
            4U,
            "alpha",
            RuntimeServiceCapabilityInvocationStatus::
                Completed,
            true,
            40U));

    traces.push_back(
        MakeTrace(
            5U,
            "beta",
            RuntimeServiceCapabilityInvocationStatus::
                InvocationRejected,
            false,
            50U));

    /*
     * The unfiltered summary counts every terminal status.
     */
    const auto all =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    REQUIRE(all.matched_count == 5U);

    REQUIRE(
        Count(
            all.summary,
            RuntimeServiceCapabilityInvocationStatus::
                Completed) ==
        2U);

    REQUIRE(
        Count(
            all.summary,
            RuntimeServiceCapabilityInvocationStatus::
                NegotiationRejected) ==
        1U);

    REQUIRE(
        Count(
            all.summary,
            RuntimeServiceCapabilityInvocationStatus::
                InvocationRejected) ==
        1U);

    REQUIRE(
        Count(
            all.summary,
            RuntimeServiceCapabilityInvocationStatus::
                PolicyRejected) ==
        1U);

    REQUIRE(
        Count(
            all.summary,
            RuntimeServiceCapabilityInvocationStatus::
                NoAttempt) ==
        0U);

    REQUIRE(
        Count(
            all.summary,
            RuntimeServiceCapabilityInvocationStatus::
                CapabilityInvokerUnavailable) ==
        0U);

    /*
     * Filtering defines the aggregation domain.
     */
    RuntimeServiceCapabilityTraceQuery alpha_query;
    alpha_query.capability = "alpha";

    const auto alpha =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            alpha_query);

    REQUIRE(alpha.matched_count == 3U);

    REQUIRE(
        Count(
            alpha.summary,
            RuntimeServiceCapabilityInvocationStatus::
                Completed) ==
        2U);

    REQUIRE(
        Count(
            alpha.summary,
            RuntimeServiceCapabilityInvocationStatus::
                PolicyRejected) ==
        1U);

    REQUIRE(
        Count(
            alpha.summary,
            RuntimeServiceCapabilityInvocationStatus::
                NegotiationRejected) ==
        0U);

    REQUIRE(
        Count(
            alpha.summary,
            RuntimeServiceCapabilityInvocationStatus::
                InvocationRejected) ==
        0U);

    /*
     * A result-status filter produces a single populated
     * status bucket.
     */
    RuntimeServiceCapabilityTraceQuery completed_query;

    completed_query.result_status =
        static_cast<std::uint8_t>(
            RuntimeServiceCapabilityInvocationStatus::
                Completed);

    const auto completed =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            completed_query);

    REQUIRE(completed.matched_count == 2U);
    REQUIRE(completed.summary.completed_traces == 2U);

    REQUIRE(
        Count(
            completed.summary,
            RuntimeServiceCapabilityInvocationStatus::
                Completed) ==
        2U);

    REQUIRE(
        Count(
            completed.summary,
            RuntimeServiceCapabilityInvocationStatus::
                PolicyRejected) ==
        0U);

    /*
     * Pagination changes returned traces but not status
     * distribution coverage.
     */
    RuntimeServiceCapabilityTraceQuery paged_query;

    paged_query.offset = 1U;
    paged_query.maximum_results = 1U;

    const auto paged =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            paged_query);

    REQUIRE(paged.matched_count == 5U);
    REQUIRE(paged.returned_count == 1U);

    REQUIRE(
        Count(
            paged.summary,
            RuntimeServiceCapabilityInvocationStatus::
                Completed) ==
        2U);

    REQUIRE(
        Count(
            paged.summary,
            RuntimeServiceCapabilityInvocationStatus::
                NegotiationRejected) ==
        1U);

    REQUIRE(
        Count(
            paged.summary,
            RuntimeServiceCapabilityInvocationStatus::
                InvocationRejected) ==
        1U);

    REQUIRE(
        Count(
            paged.summary,
            RuntimeServiceCapabilityInvocationStatus::
                PolicyRejected) ==
        1U);

    /*
     * Metadata-only queries still calculate the complete
     * distribution.
     */
    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.maximum_results = 0U;

    const auto metadata =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    REQUIRE(metadata.returned_count == 0U);
    REQUIRE(metadata.traces.empty());

    REQUIRE(
        metadata.execution_diagnostics.metadata_only);

    REQUIRE(
        Count(
            metadata.summary,
            RuntimeServiceCapabilityInvocationStatus::
                Completed) ==
        2U);

    REQUIRE(
        Count(
            metadata.summary,
            RuntimeServiceCapabilityInvocationStatus::
                PolicyRejected) ==
        1U);

    /*
     * Ordering changes page order but not the distribution.
     */
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
        5U);

    REQUIRE(
        Count(
            newest.summary,
            RuntimeServiceCapabilityInvocationStatus::
                Completed) ==
        2U);

    REQUIRE(
        Count(
            newest.summary,
            RuntimeServiceCapabilityInvocationStatus::
                InvocationRejected) ==
        1U);

    /*
     * The full uint8_t domain remains observable, including
     * a future or currently unknown status value.
     */
    auto unknown_status_trace =
        MakeTrace(
            6U,
            "future",
            RuntimeServiceCapabilityInvocationStatus::
                NoAttempt,
            false,
            60U);

    constexpr std::uint8_t unknown_status{255U};

    unknown_status_trace.result_status =
        unknown_status;

    traces.push_back(unknown_status_trace);

    RuntimeServiceCapabilityTraceQuery unknown_query;
    unknown_query.result_status = unknown_status;

    const auto unknown =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            unknown_query);

    REQUIRE(unknown.matched_count == 1U);
    REQUIRE(unknown.summary.ResultStatusCount(255U) == 1U);
    REQUIRE(unknown.summary.ResultStatusCount(254U) == 0U);

    /*
     * No matches leave every status bucket zero-valued.
     */
    RuntimeServiceCapabilityTraceQuery no_match_query;
    no_match_query.capability = "missing";

    const auto no_matches =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            no_match_query);

    REQUIRE(no_matches.matched_count == 0U);

    REQUIRE(
        Count(
            no_matches.summary,
            RuntimeServiceCapabilityInvocationStatus::
                Completed) ==
        0U);

    REQUIRE(
        Count(
            no_matches.summary,
            RuntimeServiceCapabilityInvocationStatus::
                PolicyRejected) ==
        0U);

    REQUIRE(
        no_matches.summary.ResultStatusCount(255U) ==
        0U);

    return 0;
}
