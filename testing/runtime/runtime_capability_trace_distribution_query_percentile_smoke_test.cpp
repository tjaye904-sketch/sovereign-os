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
     * Empty distributions retain deterministic zero-valued
     * percentile output.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        empty_traces;

    const auto empty_result =
        QueryRuntimeServiceCapabilityTraces(
            empty_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto empty =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            empty_result.summary,
            5000U);

    REQUIRE(
        empty.requested_percentile_basis_points ==
        5000U);

    REQUIRE(
        empty.effective_percentile_basis_points ==
        5000U);

    REQUIRE(empty.total_traces == 0U);
    REQUIRE(empty.nearest_rank == 0U);
    REQUIRE(empty.result_status == 0U);
    REQUIRE(empty.empty);
    REQUIRE(!empty.percentile_clamped);

    /*
     * Construct an ordered weighted distribution:
     *
     * status 2  -> 1 trace
     * status 8  -> 2 traces
     * status 11 -> 1 trace
     * status 40 -> 4 traces
     *
     * Ordered observations:
     * 2, 8, 8, 11, 40, 40, 40, 40
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        traces;

    traces.push_back(
        MakeTrace(1U, "alpha", 40U));

    traces.push_back(
        MakeTrace(2U, "alpha", 8U));

    traces.push_back(
        MakeTrace(3U, "beta", 2U));

    traces.push_back(
        MakeTrace(4U, "alpha", 40U));

    traces.push_back(
        MakeTrace(5U, "beta", 11U));

    traces.push_back(
        MakeTrace(6U, "alpha", 8U));

    traces.push_back(
        MakeTrace(7U, "alpha", 40U));

    traces.push_back(
        MakeTrace(8U, "alpha", 40U));

    const auto all_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto percentile_zero =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            all_result.summary,
            0U);

    REQUIRE(percentile_zero.total_traces == 8U);
    REQUIRE(percentile_zero.nearest_rank == 1U);
    REQUIRE(percentile_zero.result_status == 2U);
    REQUIRE(!percentile_zero.empty);
    REQUIRE(!percentile_zero.percentile_clamped);

    const auto percentile_25 =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            all_result.summary,
            2500U);

    REQUIRE(percentile_25.nearest_rank == 2U);
    REQUIRE(percentile_25.result_status == 8U);

    const auto percentile_50 =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            all_result.summary,
            5000U);

    REQUIRE(percentile_50.nearest_rank == 4U);
    REQUIRE(percentile_50.result_status == 11U);

    const auto percentile_75 =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            all_result.summary,
            7500U);

    REQUIRE(percentile_75.nearest_rank == 6U);
    REQUIRE(percentile_75.result_status == 40U);

    const auto percentile_100 =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            all_result.summary,
            10000U);

    REQUIRE(percentile_100.nearest_rank == 8U);
    REQUIRE(percentile_100.result_status == 40U);

    /*
     * Nonintegral percentile positions use the nearest-rank
     * ceiling rule.
     */

    const auto percentile_one_third =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            all_result.summary,
            3333U);

    REQUIRE(
        percentile_one_third.nearest_rank ==
        3U);

    REQUIRE(
        percentile_one_third.result_status ==
        8U);

    /*
     * Percentiles beyond 100 percent clamp to the complete
     * distribution's final ordered observation.
     */

    const auto clamped =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            all_result.summary,
            12000U);

    REQUIRE(
        clamped.requested_percentile_basis_points ==
        12000U);

    REQUIRE(
        clamped.effective_percentile_basis_points ==
        10000U);

    REQUIRE(clamped.percentile_clamped);
    REQUIRE(clamped.nearest_rank == 8U);
    REQUIRE(clamped.result_status == 40U);

    /*
     * Filtering defines the percentile aggregation domain.
     *
     * Alpha observations:
     * 8, 8, 40, 40, 40, 40
     */

    RuntimeServiceCapabilityTraceQuery alpha_query;
    alpha_query.capability = "alpha";

    const auto alpha_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            alpha_query);

    const auto alpha_median =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            alpha_result.summary,
            5000U);

    REQUIRE(alpha_result.matched_count == 6U);
    REQUIRE(alpha_median.total_traces == 6U);
    REQUIRE(alpha_median.nearest_rank == 3U);
    REQUIRE(alpha_median.result_status == 40U);

    /*
     * A result-status filter produces a single-status
     * percentile domain.
     */

    RuntimeServiceCapabilityTraceQuery status_query;
    status_query.result_status = 8U;

    const auto status_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            status_query);

    const auto status_percentile =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            status_result.summary,
            7500U);

    REQUIRE(status_result.matched_count == 2U);
    REQUIRE(status_percentile.total_traces == 2U);
    REQUIRE(status_percentile.nearest_rank == 2U);
    REQUIRE(status_percentile.result_status == 8U);

    /*
     * Pagination shapes only returned records. Percentiles
     * continue to describe the complete matched set.
     */

    RuntimeServiceCapabilityTraceQuery page_query;
    page_query.offset = 2U;
    page_query.maximum_results = 2U;

    const auto page_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            page_query);

    const auto page_median =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            page_result.summary,
            5000U);

    REQUIRE(page_result.matched_count == 8U);
    REQUIRE(page_result.returned_count == 2U);
    REQUIRE(page_result.has_more);

    REQUIRE(
        page_median.total_traces ==
        percentile_50.total_traces);

    REQUIRE(
        page_median.nearest_rank ==
        percentile_50.nearest_rank);

    REQUIRE(
        page_median.result_status ==
        percentile_50.result_status);

    /*
     * Metadata-only queries preserve the complete percentile
     * domain while returning no trace records.
     */

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.maximum_results = 0U;

    const auto metadata_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    const auto metadata_75 =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            metadata_result.summary,
            7500U);

    REQUIRE(metadata_result.matched_count == 8U);
    REQUIRE(metadata_result.returned_count == 0U);
    REQUIRE(metadata_result.traces.empty());

    REQUIRE(
        metadata_result.execution_diagnostics
            .metadata_only);

    REQUIRE(
        metadata_75.total_traces ==
        percentile_75.total_traces);

    REQUIRE(
        metadata_75.nearest_rank ==
        percentile_75.nearest_rank);

    REQUIRE(
        metadata_75.result_status ==
        percentile_75.result_status);

    /*
     * A no-match query restores the empty percentile
     * contract.
     */

    RuntimeServiceCapabilityTraceQuery missing_query;
    missing_query.capability = "missing";

    const auto missing_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            missing_query);

    const auto missing =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            missing_result.summary,
            5000U);

    REQUIRE(missing_result.matched_count == 0U);
    REQUIRE(missing.total_traces == 0U);
    REQUIRE(missing.nearest_rank == 0U);
    REQUIRE(missing.result_status == 0U);
    REQUIRE(missing.empty);

    return 0;
}
