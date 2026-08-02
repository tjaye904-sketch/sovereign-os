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
     * outlier diagnostics.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        empty_traces;

    const auto empty_result =
        QueryRuntimeServiceCapabilityTraces(
            empty_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto empty =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusOutliers(
            empty_result.summary);

    REQUIRE(empty.total_traces == 0U);
    REQUIRE(empty.represented_statuses == 0U);

    REQUIRE(empty.first_quartile_result_status == 0U);
    REQUIRE(empty.third_quartile_result_status == 0U);
    REQUIRE(empty.interquartile_status_range == 0U);

    REQUIRE(empty.lower_fence_twice_status == 0);
    REQUIRE(empty.upper_fence_twice_status == 0);

    REQUIRE(empty.lower_outlier_traces == 0U);
    REQUIRE(empty.upper_outlier_traces == 0U);
    REQUIRE(empty.total_outlier_traces == 0U);

    REQUIRE(empty.lower_outlier_statuses == 0U);
    REQUIRE(empty.upper_outlier_statuses == 0U);
    REQUIRE(empty.total_outlier_statuses == 0U);

    REQUIRE(
        empty.outlier_trace_coverage_basis_points ==
        0U);

    REQUIRE(empty.empty);
    REQUIRE(!empty.lower_outliers_present);
    REQUIRE(!empty.upper_outliers_present);
    REQUIRE(!empty.outliers_present);
    REQUIRE(!empty.lower_fence_below_status_domain);
    REQUIRE(!empty.upper_fence_above_status_domain);

    /*
     * A zero-spread single-status distribution contains no
     * outliers because every observation equals both fences.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        single_traces;

    single_traces.push_back(
        MakeTrace(1U, "single", 7U));

    single_traces.push_back(
        MakeTrace(2U, "single", 7U));

    single_traces.push_back(
        MakeTrace(3U, "single", 7U));

    const auto single_result =
        QueryRuntimeServiceCapabilityTraces(
            single_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto single =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusOutliers(
            single_result.summary);

    REQUIRE(single.total_traces == 3U);
    REQUIRE(single.represented_statuses == 1U);

    REQUIRE(single.first_quartile_result_status == 7U);
    REQUIRE(single.third_quartile_result_status == 7U);
    REQUIRE(single.interquartile_status_range == 0U);

    REQUIRE(single.lower_fence_twice_status == 14);
    REQUIRE(single.upper_fence_twice_status == 14);

    REQUIRE(single.total_outlier_traces == 0U);
    REQUIRE(single.total_outlier_statuses == 0U);

    REQUIRE(
        single.outlier_trace_coverage_basis_points ==
        0U);

    REQUIRE(!single.empty);
    REQUIRE(!single.outliers_present);

    /*
     * Nonzero IQR with one upper outlier.
     *
     * Ordered observations:
     * 1, 2, 2, 3, 3, 4, 4, 20
     *
     * Q1 = 2
     * Q3 = 4
     * IQR = 2
     *
     * Lower fence = -1
     * Upper fence = 7
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        upper_outlier_traces;

    upper_outlier_traces.push_back(
        MakeTrace(1U, "upper", 1U));

    upper_outlier_traces.push_back(
        MakeTrace(2U, "upper", 2U));

    upper_outlier_traces.push_back(
        MakeTrace(3U, "upper", 2U));

    upper_outlier_traces.push_back(
        MakeTrace(4U, "upper", 3U));

    upper_outlier_traces.push_back(
        MakeTrace(5U, "upper", 3U));

    upper_outlier_traces.push_back(
        MakeTrace(6U, "upper", 4U));

    upper_outlier_traces.push_back(
        MakeTrace(7U, "upper", 4U));

    upper_outlier_traces.push_back(
        MakeTrace(8U, "upper", 20U));

    const auto upper_result =
        QueryRuntimeServiceCapabilityTraces(
            upper_outlier_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto upper =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusOutliers(
            upper_result.summary);

    REQUIRE(upper.total_traces == 8U);
    REQUIRE(upper.represented_statuses == 5U);

    REQUIRE(upper.first_quartile_result_status == 2U);
    REQUIRE(upper.third_quartile_result_status == 4U);
    REQUIRE(upper.interquartile_status_range == 2U);

    REQUIRE(upper.lower_fence_twice_status == -2);
    REQUIRE(upper.upper_fence_twice_status == 14);

    REQUIRE(upper.lower_outlier_traces == 0U);
    REQUIRE(upper.upper_outlier_traces == 1U);
    REQUIRE(upper.total_outlier_traces == 1U);

    REQUIRE(upper.lower_outlier_statuses == 0U);
    REQUIRE(upper.upper_outlier_statuses == 1U);
    REQUIRE(upper.total_outlier_statuses == 1U);

    REQUIRE(
        upper.outlier_trace_coverage_basis_points ==
        1250U);

    REQUIRE(!upper.empty);
    REQUIRE(!upper.lower_outliers_present);
    REQUIRE(upper.upper_outliers_present);
    REQUIRE(upper.outliers_present);

    REQUIRE(upper.lower_fence_below_status_domain);
    REQUIRE(!upper.upper_fence_above_status_domain);

    /*
     * A collapsed IQR can classify both distribution tails
     * as outliers while the complete distribution retains
     * nonzero spread.
     *
     * Ordered observations:
     * 0, 10, 10, 10, 10, 10, 10, 30
     *
     * Q1 = Q3 = 10
     * Lower fence = upper fence = 10
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        collapsed_traces;

    collapsed_traces.push_back(
        MakeTrace(1U, "collapsed", 0U));

    for (std::uint64_t sequence = 2U;
         sequence <= 7U;
         ++sequence)
    {
        collapsed_traces.push_back(
            MakeTrace(sequence, "collapsed", 10U));
    }

    collapsed_traces.push_back(
        MakeTrace(8U, "collapsed", 30U));

    const auto collapsed_result =
        QueryRuntimeServiceCapabilityTraces(
            collapsed_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto collapsed =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusOutliers(
            collapsed_result.summary);

    REQUIRE(collapsed.total_traces == 8U);
    REQUIRE(collapsed.represented_statuses == 3U);

    REQUIRE(
        collapsed.first_quartile_result_status ==
        10U);

    REQUIRE(
        collapsed.third_quartile_result_status ==
        10U);

    REQUIRE(
        collapsed.interquartile_status_range ==
        0U);

    REQUIRE(collapsed.lower_fence_twice_status == 20);
    REQUIRE(collapsed.upper_fence_twice_status == 20);

    REQUIRE(collapsed.lower_outlier_traces == 1U);
    REQUIRE(collapsed.upper_outlier_traces == 1U);
    REQUIRE(collapsed.total_outlier_traces == 2U);

    REQUIRE(collapsed.lower_outlier_statuses == 1U);
    REQUIRE(collapsed.upper_outlier_statuses == 1U);
    REQUIRE(collapsed.total_outlier_statuses == 2U);

    REQUIRE(
        collapsed.outlier_trace_coverage_basis_points ==
        2500U);

    REQUIRE(collapsed.lower_outliers_present);
    REQUIRE(collapsed.upper_outliers_present);
    REQUIRE(collapsed.outliers_present);

    /*
     * Filtering defines the outlier aggregation domain.
     */

    collapsed_traces.push_back(
        MakeTrace(9U, "other", 255U));

    RuntimeServiceCapabilityTraceQuery filter_query;
    filter_query.capability = "collapsed";

    const auto filtered_result =
        QueryRuntimeServiceCapabilityTraces(
            collapsed_traces,
            filter_query);

    const auto filtered =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusOutliers(
            filtered_result.summary);

    REQUIRE(filtered_result.matched_count == 8U);
    REQUIRE(filtered.total_traces == 8U);
    REQUIRE(filtered.total_outlier_traces == 2U);

    REQUIRE(
        filtered.outlier_trace_coverage_basis_points ==
        2500U);

    /*
     * Pagination shapes returned records only. Diagnostics
     * continue to describe the complete matched set.
     */

    RuntimeServiceCapabilityTraceQuery page_query;
    page_query.capability = "collapsed";
    page_query.offset = 2U;
    page_query.maximum_results = 2U;

    const auto page_result =
        QueryRuntimeServiceCapabilityTraces(
            collapsed_traces,
            page_query);

    const auto page =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusOutliers(
            page_result.summary);

    REQUIRE(page_result.matched_count == 8U);
    REQUIRE(page_result.returned_count == 2U);
    REQUIRE(page_result.has_more);

    REQUIRE(page.total_traces == filtered.total_traces);

    REQUIRE(
        page.lower_fence_twice_status ==
        filtered.lower_fence_twice_status);

    REQUIRE(
        page.upper_fence_twice_status ==
        filtered.upper_fence_twice_status);

    REQUIRE(
        page.total_outlier_traces ==
        filtered.total_outlier_traces);

    REQUIRE(
        page.outlier_trace_coverage_basis_points ==
        filtered.outlier_trace_coverage_basis_points);

    /*
     * Metadata-only queries preserve complete matched-set
     * outlier diagnostics.
     */

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.capability = "collapsed";
    metadata_query.maximum_results = 0U;

    const auto metadata_result =
        QueryRuntimeServiceCapabilityTraces(
            collapsed_traces,
            metadata_query);

    const auto metadata =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusOutliers(
            metadata_result.summary);

    REQUIRE(metadata_result.matched_count == 8U);
    REQUIRE(metadata_result.returned_count == 0U);
    REQUIRE(metadata_result.traces.empty());

    REQUIRE(
        metadata_result.execution_diagnostics
            .metadata_only);

    REQUIRE(
        metadata.total_outlier_traces ==
        filtered.total_outlier_traces);

    REQUIRE(
        metadata.total_outlier_statuses ==
        filtered.total_outlier_statuses);

    REQUIRE(
        metadata.outliers_present ==
        filtered.outliers_present);

    /*
     * A no-match query restores the empty contract.
     */

    RuntimeServiceCapabilityTraceQuery missing_query;
    missing_query.capability = "missing";

    const auto missing_result =
        QueryRuntimeServiceCapabilityTraces(
            collapsed_traces,
            missing_query);

    const auto missing =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusOutliers(
            missing_result.summary);

    REQUIRE(missing_result.matched_count == 0U);
    REQUIRE(missing.total_traces == 0U);
    REQUIRE(missing.total_outlier_traces == 0U);
    REQUIRE(missing.empty);
    REQUIRE(!missing.outliers_present);

    return 0;
}
