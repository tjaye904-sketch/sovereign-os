#include "runtime_service_capability_trace_query.hpp"

#include <cstdint>
#include <deque>
#include <initializer_list>
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


void AddDistribution(
    std::deque<
        sovereign::RuntimeServiceCapabilityPipelineTrace>&
        traces,
    std::uint64_t& sequence,
    const char* capability,
    const std::initializer_list<unsigned int> statuses)
{
    for (const auto status : statuses)
    {
        traces.push_back(
            MakeTrace(
                sequence,
                capability,
                static_cast<std::uint8_t>(status)));

        ++sequence;
    }
}

} // namespace

int main()
{
    using namespace sovereign;

    /*
     * Empty distributions have an explicit empty shape.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        empty_traces;

    const auto empty_result =
        QueryRuntimeServiceCapabilityTraces(
            empty_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto empty =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            empty_result.summary);

    REQUIRE(empty.total_traces == 0U);
    REQUIRE(empty.represented_statuses == 0U);
    REQUIRE(empty.empty);
    REQUIRE(empty.classified);

    REQUIRE(
        empty.shape ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            empty);

    REQUIRE(!empty.single_point);
    REQUIRE(!empty.collapsed_core_with_tails);
    REQUIRE(!empty.outlier_dominated);
    REQUIRE(!empty.left_skewed);
    REQUIRE(!empty.right_skewed);
    REQUIRE(!empty.compact_symmetric);
    REQUIRE(!empty.dispersed_symmetric);
    REQUIRE(!empty.ambiguous);

    /*
     * Zero complete spread produces a single-point shape.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        single_traces;

    std::uint64_t sequence = 1U;

    AddDistribution(
        single_traces,
        sequence,
        "single",
        {7U, 7U, 7U});

    const auto single_result =
        QueryRuntimeServiceCapabilityTraces(
            single_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto single =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            single_result.summary);

    REQUIRE(single.total_traces == 3U);
    REQUIRE(single.represented_statuses == 1U);

    REQUIRE(single.minimum_result_status == 7U);
    REQUIRE(single.maximum_result_status == 7U);
    REQUIRE(single.total_status_range == 0U);

    REQUIRE(single.classified);
    REQUIRE(single.single_point);

    REQUIRE(
        single.shape ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            single_point);

    /*
     * A collapsed interquartile core with nonzero total
     * spread is classified before outlier dominance.
     *
     * Ordered:
     * 0, 10, 10, 10, 10, 10, 10, 30
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        collapsed_traces;

    sequence = 1U;

    AddDistribution(
        collapsed_traces,
        sequence,
        "collapsed",
        {0U, 10U, 10U, 10U, 10U, 10U, 10U, 30U});

    const auto collapsed_result =
        QueryRuntimeServiceCapabilityTraces(
            collapsed_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto collapsed =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            collapsed_result.summary);

    REQUIRE(collapsed.total_traces == 8U);
    REQUIRE(collapsed.represented_statuses == 3U);

    REQUIRE(collapsed.total_status_range == 30U);
    REQUIRE(collapsed.interquartile_status_range == 0U);

    REQUIRE(
        collapsed.outlier_trace_coverage_basis_points ==
        2500U);

    REQUIRE(collapsed.classified);
    REQUIRE(collapsed.collapsed_core_with_tails);
    REQUIRE(!collapsed.outlier_dominated);

    REQUIRE(
        collapsed.shape ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            collapsed_core_with_tails);

    /*
     * A noncollapsed distribution with at least 25 percent
     * outlier coverage is outlier-dominated.
     *
     * Ordered:
     * 0, 1, 2, 3, 4, 5, 20, 30
     *
     * Q1 = 1
     * Q3 = 5
     * IQR = 4
     * upper fence = 11
     *
     * 20 and 30 are upper outliers.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        outlier_traces;

    sequence = 1U;

    AddDistribution(
        outlier_traces,
        sequence,
        "outlier",
        {0U, 1U, 2U, 3U, 4U, 5U, 20U, 30U});

    const auto outlier_result =
        QueryRuntimeServiceCapabilityTraces(
            outlier_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto outlier =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            outlier_result.summary);

    REQUIRE(outlier.total_traces == 8U);
    REQUIRE(outlier.interquartile_status_range == 4U);
    REQUIRE(outlier.total_outlier_traces == 2U);

    REQUIRE(
        outlier.outlier_trace_coverage_basis_points ==
        2500U);

    REQUIRE(outlier.classified);
    REQUIRE(outlier.outlier_dominated);

    REQUIRE(
        outlier.shape ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            outlier_dominated);

    /*
     * Strong right skew without dominant outlier coverage.
     *
     * Ordered:
     * 0, 0, 0, 0, 0, 10, 20, 30
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        right_traces;

    sequence = 1U;

    AddDistribution(
        right_traces,
        sequence,
        "right",
        {0U, 0U, 0U, 0U, 0U, 10U, 20U, 30U});

    const auto right_result =
        QueryRuntimeServiceCapabilityTraces(
            right_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto right =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            right_result.summary);

    REQUIRE(right.total_traces == 8U);
    REQUIRE(right.skewness_basis_points == 10000);

    REQUIRE(right.classified);
    REQUIRE(right.right_skewed);
    REQUIRE(!right.left_skewed);

    REQUIRE(
        right.shape ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            right_skewed);

    /*
     * Strong left skew.
     *
     * Ordered:
     * 0, 10, 20, 30, 30, 30, 30, 30
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        left_traces;

    sequence = 1U;

    AddDistribution(
        left_traces,
        sequence,
        "left",
        {0U, 10U, 20U, 30U, 30U, 30U, 30U, 30U});

    const auto left_result =
        QueryRuntimeServiceCapabilityTraces(
            left_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto left =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            left_result.summary);

    REQUIRE(left.total_traces == 8U);
    REQUIRE(left.skewness_basis_points == -10000);

    REQUIRE(left.classified);
    REQUIRE(left.left_skewed);
    REQUIRE(!left.right_skewed);

    REQUIRE(
        left.shape ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            left_skewed);

    /*
     * Symmetric quartiles with a relatively narrow IQR
     * produce a compact-symmetric shape.
     *
     * Ordered:
     * 0, 10, 10, 20, 20, 30, 30, 40
     *
     * Q1 = 10
     * M  = 20
     * Q3 = 30
     * IQR = 20
     * total range = 40
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        compact_traces;

    sequence = 1U;

    AddDistribution(
        compact_traces,
        sequence,
        "compact",
        {0U, 10U, 10U, 20U, 20U, 30U, 30U, 40U});

    const auto compact_result =
        QueryRuntimeServiceCapabilityTraces(
            compact_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto compact =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            compact_result.summary);

    REQUIRE(compact.total_status_range == 40U);
    REQUIRE(compact.interquartile_status_range == 20U);
    REQUIRE(compact.skewness_basis_points == 0);

    REQUIRE(compact.classified);
    REQUIRE(compact.compact_symmetric);

    REQUIRE(
        compact.shape ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            compact_symmetric);

    /*
     * Symmetric quartiles occupying more than half the total
     * range produce a dispersed-symmetric shape.
     *
     * Ordered:
     * 0, 5, 10, 15, 20
     *
     * Q1 = 5
     * M  = 10
     * Q3 = 15
     * IQR = 10
     * total range = 20
     *
     * Boundary equality remains compact, so use:
     *
     * 0, 1, 10, 19, 20
     *
     * Q1 = 1
     * M  = 10
     * Q3 = 19
     * IQR = 18
     * total range = 20
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        dispersed_traces;

    sequence = 1U;

    AddDistribution(
        dispersed_traces,
        sequence,
        "dispersed",
        {0U, 1U, 10U, 19U, 20U});

    const auto dispersed_result =
        QueryRuntimeServiceCapabilityTraces(
            dispersed_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto dispersed =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            dispersed_result.summary);

    REQUIRE(dispersed.total_status_range == 20U);
    REQUIRE(dispersed.interquartile_status_range == 18U);
    REQUIRE(dispersed.skewness_basis_points == 0);

    REQUIRE(dispersed.classified);
    REQUIRE(dispersed.dispersed_symmetric);

    REQUIRE(
        dispersed.shape ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            dispersed_symmetric);

    /*
     * Filtering defines the complete classification domain.
     */

    right_traces.push_back(
        MakeTrace(
            sequence,
            "other",
            255U));

    RuntimeServiceCapabilityTraceQuery filter_query;
    filter_query.capability = "right";

    const auto filtered_result =
        QueryRuntimeServiceCapabilityTraces(
            right_traces,
            filter_query);

    const auto filtered =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            filtered_result.summary);

    REQUIRE(filtered_result.matched_count == 8U);
    REQUIRE(filtered.total_traces == 8U);
    REQUIRE(filtered.shape == right.shape);
    REQUIRE(filtered.skewness_basis_points == 10000);
    REQUIRE(filtered.right_skewed);

    /*
     * Pagination changes returned records but not the
     * matched-set classification.
     */

    RuntimeServiceCapabilityTraceQuery page_query;
    page_query.capability = "right";
    page_query.offset = 2U;
    page_query.maximum_results = 2U;

    const auto page_result =
        QueryRuntimeServiceCapabilityTraces(
            right_traces,
            page_query);

    const auto page =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            page_result.summary);

    REQUIRE(page_result.matched_count == 8U);
    REQUIRE(page_result.returned_count == 2U);
    REQUIRE(page_result.has_more);

    REQUIRE(page.total_traces == filtered.total_traces);
    REQUIRE(page.shape == filtered.shape);

    REQUIRE(
        page.skewness_basis_points ==
        filtered.skewness_basis_points);

    REQUIRE(
        page.outlier_trace_coverage_basis_points ==
        filtered.outlier_trace_coverage_basis_points);

    /*
     * Metadata-only shaping preserves classification.
     */

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.capability = "right";
    metadata_query.maximum_results = 0U;

    const auto metadata_result =
        QueryRuntimeServiceCapabilityTraces(
            right_traces,
            metadata_query);

    const auto metadata =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            metadata_result.summary);

    REQUIRE(metadata_result.matched_count == 8U);
    REQUIRE(metadata_result.returned_count == 0U);
    REQUIRE(metadata_result.traces.empty());

    REQUIRE(
        metadata_result.execution_diagnostics
            .metadata_only);

    REQUIRE(metadata.shape == filtered.shape);
    REQUIRE(metadata.right_skewed);
    REQUIRE(metadata.classified);

    /*
     * No-match queries restore the empty classification.
     */

    RuntimeServiceCapabilityTraceQuery missing_query;
    missing_query.capability = "missing";

    const auto missing_result =
        QueryRuntimeServiceCapabilityTraces(
            right_traces,
            missing_query);

    const auto missing =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            missing_result.summary);

    REQUIRE(missing_result.matched_count == 0U);
    REQUIRE(missing.total_traces == 0U);
    REQUIRE(missing.empty);
    REQUIRE(missing.classified);

    REQUIRE(
        missing.shape ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            empty);

    return 0;
}
