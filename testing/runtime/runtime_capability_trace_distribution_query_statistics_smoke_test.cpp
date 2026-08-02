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
    const std::uint8_t result_status)
{
    sovereign::RuntimeServiceCapabilityPipelineTrace
        trace;

    trace.invocation_sequence = sequence;
    trace.capability = capability;
    trace.operation = "execute";
    trace.selected_service_name = "provider";
    trace.result_status = result_status;

    return trace;
}

} // namespace

int main()
{
    using namespace sovereign;

    /*
     * An empty result has deterministic zero-valued
     * statistics.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        empty_traces;

    const auto empty_result =
        QueryRuntimeServiceCapabilityTraces(
            empty_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto empty =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusDistribution(
            empty_result.summary);

    REQUIRE(empty.total_traces == 0U);
    REQUIRE(empty.represented_statuses == 0U);
    REQUIRE(empty.minimum_result_status == 0U);
    REQUIRE(empty.maximum_result_status == 0U);
    REQUIRE(empty.result_status_span == 0U);
    REQUIRE(empty.dominant_trace_count == 0U);
    REQUIRE(empty.dominant_statuses == 0U);
    REQUIRE(empty.empty);
    REQUIRE(!empty.single_status);

    /*
     * The full uint8_t result-status domain remains
     * observable.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        traces;

    traces.push_back(
        MakeTrace(1U, "alpha", 4U));

    traces.push_back(
        MakeTrace(2U, "beta", 0U));

    traces.push_back(
        MakeTrace(3U, "alpha", 255U));

    traces.push_back(
        MakeTrace(4U, "alpha", 4U));

    traces.push_back(
        MakeTrace(5U, "beta", 17U));

    const auto all =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto all_statistics =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusDistribution(
            all.summary);

    REQUIRE(all.matched_count == 5U);

    REQUIRE(all_statistics.total_traces == 5U);
    REQUIRE(all_statistics.represented_statuses == 4U);

    REQUIRE(
        all_statistics.minimum_result_status ==
        0U);

    REQUIRE(
        all_statistics.maximum_result_status ==
        255U);

    REQUIRE(
        all_statistics.result_status_span ==
        255U);

    REQUIRE(
        all_statistics.dominant_trace_count ==
        2U);

    REQUIRE(
        all_statistics.dominant_statuses ==
        1U);

    REQUIRE(!all_statistics.empty);
    REQUIRE(!all_statistics.single_status);

    /*
     * Filtering defines the statistical aggregation domain.
     */

    RuntimeServiceCapabilityTraceQuery alpha_query;
    alpha_query.capability = "alpha";

    const auto alpha =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            alpha_query);

    const auto alpha_statistics =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusDistribution(
            alpha.summary);

    REQUIRE(alpha.matched_count == 3U);
    REQUIRE(alpha_statistics.total_traces == 3U);
    REQUIRE(alpha_statistics.represented_statuses == 2U);

    REQUIRE(
        alpha_statistics.minimum_result_status ==
        4U);

    REQUIRE(
        alpha_statistics.maximum_result_status ==
        255U);

    REQUIRE(
        alpha_statistics.result_status_span ==
        251U);

    REQUIRE(
        alpha_statistics.dominant_trace_count ==
        2U);

    REQUIRE(
        alpha_statistics.dominant_statuses ==
        1U);

    /*
     * A result-status filter produces one represented status
     * and a zero-width status range.
     */

    RuntimeServiceCapabilityTraceQuery status_query;
    status_query.result_status = 4U;

    const auto status_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            status_query);

    const auto status_statistics =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusDistribution(
            status_result.summary);

    REQUIRE(status_result.matched_count == 2U);

    REQUIRE(
        status_statistics.total_traces ==
        2U);

    REQUIRE(
        status_statistics.represented_statuses ==
        1U);

    REQUIRE(
        status_statistics.minimum_result_status ==
        4U);

    REQUIRE(
        status_statistics.maximum_result_status ==
        4U);

    REQUIRE(
        status_statistics.result_status_span ==
        0U);

    REQUIRE(
        status_statistics.dominant_trace_count ==
        2U);

    REQUIRE(
        status_statistics.dominant_statuses ==
        1U);

    REQUIRE(status_statistics.single_status);

    /*
     * Equal bucket counts identify every tied bucket as
     * dominant.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        uniform_traces;

    uniform_traces.push_back(
        MakeTrace(1U, "uniform", 2U));

    uniform_traces.push_back(
        MakeTrace(2U, "uniform", 8U));

    uniform_traces.push_back(
        MakeTrace(3U, "uniform", 11U));

    const auto uniform_result =
        QueryRuntimeServiceCapabilityTraces(
            uniform_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto uniform_statistics =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusDistribution(
            uniform_result.summary);

    REQUIRE(uniform_statistics.total_traces == 3U);
    REQUIRE(uniform_statistics.represented_statuses == 3U);

    REQUIRE(
        uniform_statistics.dominant_trace_count ==
        1U);

    REQUIRE(
        uniform_statistics.dominant_statuses ==
        3U);

    REQUIRE(
        uniform_statistics.result_status_span ==
        9U);

    /*
     * Pagination and metadata-only shaping do not change
     * statistics for the complete matched result set.
     */

    RuntimeServiceCapabilityTraceQuery page_query;
    page_query.offset = 1U;
    page_query.maximum_results = 1U;

    const auto page =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            page_query);

    const auto page_statistics =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusDistribution(
            page.summary);

    REQUIRE(page.returned_count == 1U);
    REQUIRE(page.has_more);

    REQUIRE(
        page_statistics.total_traces ==
        all_statistics.total_traces);

    REQUIRE(
        page_statistics.represented_statuses ==
        all_statistics.represented_statuses);

    REQUIRE(
        page_statistics.result_status_span ==
        all_statistics.result_status_span);

    REQUIRE(
        page_statistics.dominant_trace_count ==
        all_statistics.dominant_trace_count);

    REQUIRE(
        page_statistics.dominant_statuses ==
        all_statistics.dominant_statuses);

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.maximum_results = 0U;

    const auto metadata =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    const auto metadata_statistics =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusDistribution(
            metadata.summary);

    REQUIRE(metadata.returned_count == 0U);
    REQUIRE(metadata.traces.empty());

    REQUIRE(
        metadata.execution_diagnostics
            .metadata_only);

    REQUIRE(
        metadata_statistics.total_traces ==
        all_statistics.total_traces);

    REQUIRE(
        metadata_statistics.result_status_span ==
        all_statistics.result_status_span);

    /*
     * A no-match query restores the empty statistics
     * contract.
     */

    RuntimeServiceCapabilityTraceQuery missing_query;
    missing_query.capability = "missing";

    const auto missing =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            missing_query);

    const auto missing_statistics =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusDistribution(
            missing.summary);

    REQUIRE(missing.matched_count == 0U);
    REQUIRE(missing_statistics.total_traces == 0U);
    REQUIRE(missing_statistics.represented_statuses == 0U);
    REQUIRE(missing_statistics.empty);
    REQUIRE(!missing_statistics.single_status);

    return 0;
}
