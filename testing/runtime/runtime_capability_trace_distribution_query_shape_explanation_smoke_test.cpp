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
    sovereign::RuntimeServiceCapabilityPipelineTrace trace;

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

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        traces;

    /*
     * Empty distributions are explicitly explained.
     */

    auto result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    auto explanation =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
            result.summary);

    REQUIRE(explanation.explanation_available);
    REQUIRE(explanation.matched_set_stable);
    REQUIRE(explanation.precedence_rank == 1U);
    REQUIRE(explanation.structurally_conclusive);
    REQUIRE(!explanation.confidence_limited);

    REQUIRE(
        explanation.governing_rule ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
            empty_distribution);

    /*
     * Zero spread is structurally conclusive.
     */

    std::uint64_t sequence = 1U;

    AddDistribution(
        traces,
        sequence,
        "single",
        {7U, 7U, 7U});

    RuntimeServiceCapabilityTraceQuery query;
    query.capability = "single";

    result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            query);

    const auto single =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
            result.summary);

    REQUIRE(single.precedence_rank == 2U);
    REQUIRE(single.structurally_conclusive);
    REQUIRE(single.confidence_basis_points == 10000U);

    REQUIRE(
        single.governing_rule ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
            zero_complete_spread);

    /*
     * Collapsed IQR precedes outlier dominance.
     */

    AddDistribution(
        traces,
        sequence,
        "collapsed",
        {0U, 10U, 10U, 10U, 10U, 10U, 10U, 30U});

    query.capability = "collapsed";

    result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            query);

    const auto collapsed =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
            result.summary);

    REQUIRE(collapsed.precedence_rank == 3U);
    REQUIRE(collapsed.structurally_conclusive);
    REQUIRE(!collapsed.threshold_governed);

    REQUIRE(
        collapsed.governing_rule ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
            collapsed_interquartile_core);

    /*
     * Exact outlier threshold is boundary-adjacent.
     */

    AddDistribution(
        traces,
        sequence,
        "outlier",
        {0U, 1U, 2U, 3U, 4U, 5U, 20U, 30U});

    query.capability = "outlier";

    result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            query);

    const auto outlier =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
            result.summary);

    REQUIRE(outlier.precedence_rank == 4U);
    REQUIRE(outlier.threshold_governed);
    REQUIRE(outlier.boundary_adjacent);
    REQUIRE(outlier.confidence_limited);

    REQUIRE(
        outlier.governing_rule ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
            dominant_outlier_coverage);

    /*
     * Directional skew has precedence rank five.
     */

    AddDistribution(
        traces,
        sequence,
        "right",
        {0U, 0U, 0U, 0U, 0U, 10U, 20U, 30U});

    query.capability = "right";

    result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            query);

    const auto right =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
            result.summary);

    REQUIRE(right.precedence_rank == 5U);
    REQUIRE(right.directional_evidence);
    REQUIRE(!right.symmetry_evidence);
    REQUIRE(right.confidence_basis_points == 10000U);

    REQUIRE(
        right.governing_rule ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
            right_quartile_skew);

    /*
     * Compact symmetry at the spread boundary is limited.
     */

    AddDistribution(
        traces,
        sequence,
        "compact",
        {0U, 10U, 10U, 20U, 20U, 30U, 30U, 40U});

    query.capability = "compact";

    result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            query);

    const auto compact =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
            result.summary);

    REQUIRE(compact.precedence_rank == 6U);
    REQUIRE(compact.symmetry_evidence);
    REQUIRE(compact.boundary_adjacent);
    REQUIRE(compact.confidence_limited);

    REQUIRE(
        compact.governing_rule ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
            compact_quartile_symmetry);

    /*
     * Small samples expose the confidence cap.
     */

    AddDistribution(
        traces,
        sequence,
        "dispersed",
        {0U, 1U, 10U, 19U, 20U});

    query.capability = "dispersed";

    result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            query);

    const auto dispersed =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
            result.summary);

    REQUIRE(dispersed.precedence_rank == 6U);
    REQUIRE(dispersed.symmetry_evidence);
    REQUIRE(dispersed.sample_limited);
    REQUIRE(dispersed.confidence_limited);

    REQUIRE(
        dispersed.governing_rule ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
            dispersed_quartile_symmetry);

    /*
     * Pagination and metadata-only shaping preserve the
     * complete matched-set explanation.
     */

    RuntimeServiceCapabilityTraceQuery filtered_query;
    filtered_query.capability = "right";

    const auto filtered_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            filtered_query);

    const auto filtered =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
            filtered_result.summary);

    RuntimeServiceCapabilityTraceQuery page_query;
    page_query.capability = "right";
    page_query.offset = 2U;
    page_query.maximum_results = 2U;

    const auto page_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            page_query);

    const auto page =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
            page_result.summary);

    REQUIRE(page_result.matched_count == 8U);
    REQUIRE(page_result.returned_count == 2U);

    REQUIRE(page.shape == filtered.shape);
    REQUIRE(page.confidence == filtered.confidence);
    REQUIRE(page.governing_rule == filtered.governing_rule);
    REQUIRE(page.precedence_rank == filtered.precedence_rank);

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.capability = "right";
    metadata_query.maximum_results = 0U;

    const auto metadata_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    const auto metadata =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
            metadata_result.summary);

    REQUIRE(metadata_result.matched_count == 8U);
    REQUIRE(metadata_result.returned_count == 0U);
    REQUIRE(metadata_result.traces.empty());

    REQUIRE(metadata.governing_rule == filtered.governing_rule);
    REQUIRE(metadata.confidence_basis_points == 10000U);
    REQUIRE(metadata.matched_set_stable);

    return 0;
}
