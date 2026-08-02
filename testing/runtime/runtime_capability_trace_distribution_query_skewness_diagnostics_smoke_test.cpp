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

void AddTrace(
    std::deque<
        sovereign::RuntimeServiceCapabilityPipelineTrace>&
        traces,
    std::uint64_t& sequence,
    const char* capability,
    const std::uint8_t result_status)
{
    traces.push_back(
        MakeTrace(
            sequence,
            capability,
            result_status));

    ++sequence;
}

} // namespace

int main()
{
    using namespace sovereign;

    /*
     * Empty distributions retain the deterministic empty
     * skewness contract.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        empty_traces;

    const auto empty_result =
        QueryRuntimeServiceCapabilityTraces(
            empty_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto empty =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusSkewness(
            empty_result.summary);

    REQUIRE(empty.total_traces == 0U);

    REQUIRE(empty.first_quartile_result_status == 0U);
    REQUIRE(empty.median_result_status == 0U);
    REQUIRE(empty.third_quartile_result_status == 0U);

    REQUIRE(empty.interquartile_status_range == 0U);
    REQUIRE(empty.skewness_numerator == 0);
    REQUIRE(empty.skewness_basis_points == 0);

    REQUIRE(empty.empty);
    REQUIRE(!empty.skewness_defined);

    REQUIRE(!empty.symmetric);
    REQUIRE(!empty.left_skewed);
    REQUIRE(!empty.right_skewed);

    REQUIRE(!empty.slight_skew);
    REQUIRE(!empty.moderate_skew);
    REQUIRE(!empty.strong_skew);

    /*
     * A collapsed IQR cannot define Bowley skewness.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        collapsed_traces;

    std::uint64_t sequence = 1U;

    AddTrace(
        collapsed_traces,
        sequence,
        "collapsed",
        1U);

    AddTrace(
        collapsed_traces,
        sequence,
        "collapsed",
        5U);

    AddTrace(
        collapsed_traces,
        sequence,
        "collapsed",
        5U);

    AddTrace(
        collapsed_traces,
        sequence,
        "collapsed",
        5U);

    AddTrace(
        collapsed_traces,
        sequence,
        "collapsed",
        9U);

    const auto collapsed_result =
        QueryRuntimeServiceCapabilityTraces(
            collapsed_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto collapsed =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusSkewness(
            collapsed_result.summary);

    REQUIRE(collapsed.total_traces == 5U);

    REQUIRE(
        collapsed.first_quartile_result_status ==
        5U);

    REQUIRE(
        collapsed.median_result_status ==
        5U);

    REQUIRE(
        collapsed.third_quartile_result_status ==
        5U);

    REQUIRE(
        collapsed.interquartile_status_range ==
        0U);

    REQUIRE(collapsed.skewness_numerator == 0);
    REQUIRE(collapsed.skewness_basis_points == 0);

    REQUIRE(!collapsed.empty);
    REQUIRE(!collapsed.skewness_defined);
    REQUIRE(!collapsed.symmetric);

    /*
     * Symmetric quartiles:
     *
     * Ordered observations:
     * 0, 10, 20, 30, 40
     *
     * Q1 = 10
     * M  = 20
     * Q3 = 30
     *
     * Numerator = 30 + 10 - 40 = 0
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        symmetric_traces;

    sequence = 1U;

    for (const auto status :
         {0U, 10U, 20U, 30U, 40U})
    {
        AddTrace(
            symmetric_traces,
            sequence,
            "symmetric",
            static_cast<std::uint8_t>(status));
    }

    const auto symmetric_result =
        QueryRuntimeServiceCapabilityTraces(
            symmetric_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto symmetric =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusSkewness(
            symmetric_result.summary);

    REQUIRE(symmetric.total_traces == 5U);

    REQUIRE(
        symmetric.first_quartile_result_status ==
        10U);

    REQUIRE(
        symmetric.median_result_status ==
        20U);

    REQUIRE(
        symmetric.third_quartile_result_status ==
        30U);

    REQUIRE(
        symmetric.interquartile_status_range ==
        20U);

    REQUIRE(symmetric.skewness_numerator == 0);
    REQUIRE(symmetric.skewness_basis_points == 0);

    REQUIRE(symmetric.skewness_defined);
    REQUIRE(symmetric.symmetric);

    REQUIRE(!symmetric.left_skewed);
    REQUIRE(!symmetric.right_skewed);

    REQUIRE(
        symmetric.direction ==
        RuntimeServiceCapabilityTraceResultStatusSkewDirection::
            symmetric);

    REQUIRE(
        symmetric.magnitude ==
        RuntimeServiceCapabilityTraceResultStatusSkewMagnitude::
            none);

    /*
     * Strong right skew:
     *
     * Ordered observations:
     * 0, 0, 0, 0, 0, 10, 20, 30
     *
     * Q1 = 0
     * M  = 0
     * Q3 = 10
     *
     * Numerator = 10
     * IQR       = 10
     * Skewness  = +10000 bp
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        right_traces;

    sequence = 1U;

    for (const auto status :
         {0U, 0U, 0U, 0U, 0U, 10U, 20U, 30U})
    {
        AddTrace(
            right_traces,
            sequence,
            "right",
            static_cast<std::uint8_t>(status));
    }

    const auto right_result =
        QueryRuntimeServiceCapabilityTraces(
            right_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto right =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusSkewness(
            right_result.summary);

    REQUIRE(right.total_traces == 8U);

    REQUIRE(right.first_quartile_result_status == 0U);
    REQUIRE(right.median_result_status == 0U);
    REQUIRE(right.third_quartile_result_status == 10U);

    REQUIRE(right.interquartile_status_range == 10U);
    REQUIRE(right.skewness_numerator == 10);
    REQUIRE(right.skewness_basis_points == 10000);

    REQUIRE(right.skewness_defined);
    REQUIRE(!right.symmetric);
    REQUIRE(!right.left_skewed);
    REQUIRE(right.right_skewed);

    REQUIRE(!right.slight_skew);
    REQUIRE(!right.moderate_skew);
    REQUIRE(right.strong_skew);

    REQUIRE(
        right.direction ==
        RuntimeServiceCapabilityTraceResultStatusSkewDirection::
            right);

    REQUIRE(
        right.magnitude ==
        RuntimeServiceCapabilityTraceResultStatusSkewMagnitude::
            strong);

    /*
     * Strong left skew:
     *
     * Ordered observations:
     * 0, 10, 20, 30, 30, 30, 30, 30
     *
     * Q1 = 10
     * M  = 30
     * Q3 = 30
     *
     * Numerator = 30 + 10 - 60 = -20
     * IQR       = 20
     * Skewness  = -10000 bp
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        left_traces;

    sequence = 1U;

    for (const auto status :
         {0U, 10U, 20U, 30U, 30U, 30U, 30U, 30U})
    {
        AddTrace(
            left_traces,
            sequence,
            "left",
            static_cast<std::uint8_t>(status));
    }

    const auto left_result =
        QueryRuntimeServiceCapabilityTraces(
            left_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto left =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusSkewness(
            left_result.summary);

    REQUIRE(left.total_traces == 8U);

    REQUIRE(left.first_quartile_result_status == 10U);
    REQUIRE(left.median_result_status == 30U);
    REQUIRE(left.third_quartile_result_status == 30U);

    REQUIRE(left.interquartile_status_range == 20U);
    REQUIRE(left.skewness_numerator == -20);
    REQUIRE(left.skewness_basis_points == -10000);

    REQUIRE(left.skewness_defined);
    REQUIRE(!left.symmetric);
    REQUIRE(left.left_skewed);
    REQUIRE(!left.right_skewed);

    REQUIRE(left.strong_skew);

    REQUIRE(
        left.direction ==
        RuntimeServiceCapabilityTraceResultStatusSkewDirection::
            left);

    /*
     * Moderate right skew:
     *
     * Q1 = 10
     * M  = 15
     * Q3 = 30
     *
     * numerator = 10
     * IQR       = 20
     * skewness  = 5000 bp
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        moderate_traces;

    sequence = 1U;

    for (const auto status :
         {0U, 10U, 10U, 15U, 15U, 30U, 30U, 40U})
    {
        AddTrace(
            moderate_traces,
            sequence,
            "moderate",
            static_cast<std::uint8_t>(status));
    }

    const auto moderate_result =
        QueryRuntimeServiceCapabilityTraces(
            moderate_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto moderate =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusSkewness(
            moderate_result.summary);

    REQUIRE(
        moderate.first_quartile_result_status ==
        10U);

    REQUIRE(
        moderate.median_result_status ==
        15U);

    REQUIRE(
        moderate.third_quartile_result_status ==
        30U);

    REQUIRE(
        moderate.interquartile_status_range ==
        20U);

    REQUIRE(moderate.skewness_numerator == 10);
    REQUIRE(moderate.skewness_basis_points == 5000);

    REQUIRE(moderate.right_skewed);
    REQUIRE(moderate.strong_skew);

    /*
     * Filtering defines the skewness diagnostic domain.
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusSkewness(
            filtered_result.summary);

    REQUIRE(filtered_result.matched_count == 8U);
    REQUIRE(filtered.total_traces == 8U);

    REQUIRE(
        filtered.skewness_basis_points ==
        right.skewness_basis_points);

    REQUIRE(filtered.right_skewed);
    REQUIRE(filtered.strong_skew);

    /*
     * Pagination shapes returned records only. Skewness
     * remains derived from the complete matched set.
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusSkewness(
            page_result.summary);

    REQUIRE(page_result.matched_count == 8U);
    REQUIRE(page_result.returned_count == 2U);
    REQUIRE(page_result.has_more);

    REQUIRE(page.total_traces == filtered.total_traces);

    REQUIRE(
        page.first_quartile_result_status ==
        filtered.first_quartile_result_status);

    REQUIRE(
        page.median_result_status ==
        filtered.median_result_status);

    REQUIRE(
        page.third_quartile_result_status ==
        filtered.third_quartile_result_status);

    REQUIRE(
        page.skewness_basis_points ==
        filtered.skewness_basis_points);

    REQUIRE(page.direction == filtered.direction);
    REQUIRE(page.magnitude == filtered.magnitude);

    /*
     * Metadata-only queries retain complete matched-set
     * skewness diagnostics.
     */

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.capability = "right";
    metadata_query.maximum_results = 0U;

    const auto metadata_result =
        QueryRuntimeServiceCapabilityTraces(
            right_traces,
            metadata_query);

    const auto metadata =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusSkewness(
            metadata_result.summary);

    REQUIRE(metadata_result.matched_count == 8U);
    REQUIRE(metadata_result.returned_count == 0U);
    REQUIRE(metadata_result.traces.empty());

    REQUIRE(
        metadata_result.execution_diagnostics
            .metadata_only);

    REQUIRE(
        metadata.skewness_basis_points ==
        filtered.skewness_basis_points);

    REQUIRE(
        metadata.skewness_defined ==
        filtered.skewness_defined);

    REQUIRE(
        metadata.right_skewed ==
        filtered.right_skewed);

    /*
     * A no-match query restores the empty contract.
     */

    RuntimeServiceCapabilityTraceQuery missing_query;
    missing_query.capability = "missing";

    const auto missing_result =
        QueryRuntimeServiceCapabilityTraces(
            right_traces,
            missing_query);

    const auto missing =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusSkewness(
            missing_result.summary);

    REQUIRE(missing_result.matched_count == 0U);
    REQUIRE(missing.total_traces == 0U);
    REQUIRE(missing.empty);
    REQUIRE(!missing.skewness_defined);
    REQUIRE(!missing.symmetric);
    REQUIRE(!missing.left_skewed);
    REQUIRE(!missing.right_skewed);

    return 0;
}
