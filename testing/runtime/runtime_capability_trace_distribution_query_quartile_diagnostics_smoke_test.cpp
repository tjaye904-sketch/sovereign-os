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
     * Empty distributions produce deterministic zero-valued
     * quartile diagnostics.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        empty_traces;

    const auto empty_result =
        QueryRuntimeServiceCapabilityTraces(
            empty_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto empty =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusQuartiles(
            empty_result.summary);

    REQUIRE(empty.total_traces == 0U);

    REQUIRE(empty.minimum_result_status == 0U);
    REQUIRE(empty.first_quartile_result_status == 0U);
    REQUIRE(empty.median_result_status == 0U);
    REQUIRE(empty.third_quartile_result_status == 0U);
    REQUIRE(empty.maximum_result_status == 0U);

    REQUIRE(empty.interquartile_status_range == 0U);
    REQUIRE(empty.lower_tail_status_range == 0U);
    REQUIRE(empty.upper_tail_status_range == 0U);

    REQUIRE(empty.empty);
    REQUIRE(!empty.zero_spread);
    REQUIRE(!empty.lower_quartile_collapsed);
    REQUIRE(!empty.interquartile_collapsed);
    REQUIRE(!empty.upper_quartile_collapsed);
    REQUIRE(!empty.all_quartiles_collapsed);

    /*
     * A single-status distribution has zero spread and all
     * quartile boundaries collapse to the same status.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        single_status_traces;

    single_status_traces.push_back(
        MakeTrace(1U, "single", 7U));

    single_status_traces.push_back(
        MakeTrace(2U, "single", 7U));

    single_status_traces.push_back(
        MakeTrace(3U, "single", 7U));

    const auto single_result =
        QueryRuntimeServiceCapabilityTraces(
            single_status_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto single =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusQuartiles(
            single_result.summary);

    REQUIRE(single.total_traces == 3U);

    REQUIRE(single.minimum_result_status == 7U);
    REQUIRE(single.first_quartile_result_status == 7U);
    REQUIRE(single.median_result_status == 7U);
    REQUIRE(single.third_quartile_result_status == 7U);
    REQUIRE(single.maximum_result_status == 7U);

    REQUIRE(single.interquartile_status_range == 0U);
    REQUIRE(single.lower_tail_status_range == 0U);
    REQUIRE(single.upper_tail_status_range == 0U);

    REQUIRE(!single.empty);
    REQUIRE(single.zero_spread);
    REQUIRE(single.lower_quartile_collapsed);
    REQUIRE(single.interquartile_collapsed);
    REQUIRE(single.upper_quartile_collapsed);
    REQUIRE(single.all_quartiles_collapsed);

    /*
     * Weighted distribution:
     *
     * status 2  -> 1 trace
     * status 8  -> 2 traces
     * status 11 -> 1 trace
     * status 40 -> 4 traces
     *
     * Ordered observations:
     * 2, 8, 8, 11, 40, 40, 40, 40
     *
     * Five-number summary:
     * minimum = 2
     * Q1      = 8
     * median  = 11
     * Q3      = 40
     * maximum = 40
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

    const auto all =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusQuartiles(
            all_result.summary);

    REQUIRE(all.total_traces == 8U);

    REQUIRE(all.minimum_result_status == 2U);
    REQUIRE(all.first_quartile_result_status == 8U);
    REQUIRE(all.median_result_status == 11U);
    REQUIRE(all.third_quartile_result_status == 40U);
    REQUIRE(all.maximum_result_status == 40U);

    REQUIRE(all.interquartile_status_range == 32U);
    REQUIRE(all.lower_tail_status_range == 6U);
    REQUIRE(all.upper_tail_status_range == 0U);

    REQUIRE(!all.empty);
    REQUIRE(!all.zero_spread);
    REQUIRE(!all.lower_quartile_collapsed);
    REQUIRE(!all.interquartile_collapsed);
    REQUIRE(all.upper_quartile_collapsed);
    REQUIRE(!all.all_quartiles_collapsed);

    /*
     * Quartiles may collapse even when the complete
     * distribution still has nonzero spread.
     *
     * Ordered observations:
     * 1, 5, 5, 5, 9
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        concentrated_traces;

    concentrated_traces.push_back(
        MakeTrace(1U, "concentrated", 1U));

    concentrated_traces.push_back(
        MakeTrace(2U, "concentrated", 5U));

    concentrated_traces.push_back(
        MakeTrace(3U, "concentrated", 5U));

    concentrated_traces.push_back(
        MakeTrace(4U, "concentrated", 5U));

    concentrated_traces.push_back(
        MakeTrace(5U, "concentrated", 9U));

    const auto concentrated_result =
        QueryRuntimeServiceCapabilityTraces(
            concentrated_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto concentrated =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusQuartiles(
            concentrated_result.summary);

    REQUIRE(concentrated.total_traces == 5U);

    REQUIRE(
        concentrated.minimum_result_status ==
        1U);

    REQUIRE(
        concentrated.first_quartile_result_status ==
        5U);

    REQUIRE(
        concentrated.median_result_status ==
        5U);

    REQUIRE(
        concentrated.third_quartile_result_status ==
        5U);

    REQUIRE(
        concentrated.maximum_result_status ==
        9U);

    REQUIRE(
        concentrated.interquartile_status_range ==
        0U);

    REQUIRE(
        concentrated.lower_tail_status_range ==
        4U);

    REQUIRE(
        concentrated.upper_tail_status_range ==
        4U);

    REQUIRE(!concentrated.zero_spread);
    REQUIRE(!concentrated.lower_quartile_collapsed);
    REQUIRE(concentrated.interquartile_collapsed);
    REQUIRE(!concentrated.upper_quartile_collapsed);
    REQUIRE(concentrated.all_quartiles_collapsed);

    /*
     * Filtering defines the quartile diagnostic domain.
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

    const auto alpha =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusQuartiles(
            alpha_result.summary);

    REQUIRE(alpha_result.matched_count == 6U);
    REQUIRE(alpha.total_traces == 6U);

    REQUIRE(alpha.minimum_result_status == 8U);
    REQUIRE(alpha.first_quartile_result_status == 8U);
    REQUIRE(alpha.median_result_status == 40U);
    REQUIRE(alpha.third_quartile_result_status == 40U);
    REQUIRE(alpha.maximum_result_status == 40U);

    REQUIRE(alpha.interquartile_status_range == 32U);
    REQUIRE(alpha.lower_tail_status_range == 0U);
    REQUIRE(alpha.upper_tail_status_range == 0U);

    REQUIRE(alpha.lower_quartile_collapsed);
    REQUIRE(!alpha.interquartile_collapsed);
    REQUIRE(alpha.upper_quartile_collapsed);
    REQUIRE(!alpha.all_quartiles_collapsed);

    /*
     * Pagination changes returned records but not quartile
     * diagnostics for the complete matched set.
     */

    RuntimeServiceCapabilityTraceQuery page_query;
    page_query.offset = 2U;
    page_query.maximum_results = 2U;

    const auto page_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            page_query);

    const auto page =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusQuartiles(
            page_result.summary);

    REQUIRE(page_result.matched_count == 8U);
    REQUIRE(page_result.returned_count == 2U);
    REQUIRE(page_result.has_more);

    REQUIRE(page.total_traces == all.total_traces);

    REQUIRE(
        page.minimum_result_status ==
        all.minimum_result_status);

    REQUIRE(
        page.first_quartile_result_status ==
        all.first_quartile_result_status);

    REQUIRE(
        page.median_result_status ==
        all.median_result_status);

    REQUIRE(
        page.third_quartile_result_status ==
        all.third_quartile_result_status);

    REQUIRE(
        page.maximum_result_status ==
        all.maximum_result_status);

    REQUIRE(
        page.interquartile_status_range ==
        all.interquartile_status_range);

    /*
     * Metadata-only result shaping also preserves complete
     * matched-set quartile diagnostics.
     */

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.maximum_results = 0U;

    const auto metadata_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    const auto metadata =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusQuartiles(
            metadata_result.summary);

    REQUIRE(metadata_result.matched_count == 8U);
    REQUIRE(metadata_result.returned_count == 0U);
    REQUIRE(metadata_result.traces.empty());

    REQUIRE(
        metadata_result.execution_diagnostics
            .metadata_only);

    REQUIRE(metadata.total_traces == all.total_traces);

    REQUIRE(
        metadata.median_result_status ==
        all.median_result_status);

    REQUIRE(
        metadata.interquartile_status_range ==
        all.interquartile_status_range);

    REQUIRE(
        metadata.upper_quartile_collapsed ==
        all.upper_quartile_collapsed);

    /*
     * A no-match query restores the empty diagnostics
     * contract.
     */

    RuntimeServiceCapabilityTraceQuery missing_query;
    missing_query.capability = "missing";

    const auto missing_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            missing_query);

    const auto missing =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusQuartiles(
            missing_result.summary);

    REQUIRE(missing_result.matched_count == 0U);
    REQUIRE(missing.total_traces == 0U);
    REQUIRE(missing.empty);
    REQUIRE(!missing.zero_spread);
    REQUIRE(!missing.all_quartiles_collapsed);

    return 0;
}
