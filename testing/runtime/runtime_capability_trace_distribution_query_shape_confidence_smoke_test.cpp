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

    /*
     * Empty distributions have undefined confidence.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        traces;

    auto result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    auto confidence =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
            result.summary);

    REQUIRE(confidence.total_traces == 0U);
    REQUIRE(!confidence.confidence_defined);
    REQUIRE(confidence.confidence_basis_points == 0U);

    REQUIRE(
        confidence.confidence ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence::
            undefined);

    /*
     * A single-point distribution is structurally
     * definitive despite having fewer than four traces.
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
            result.summary);

    REQUIRE(single.total_traces == 3U);
    REQUIRE(single.confidence_defined);
    REQUIRE(!single.sample_limited);

    REQUIRE(single.confidence_basis_points == 10000U);
    REQUIRE(single.evidence_margin_basis_points == 10000U);

    REQUIRE(single.definitive_confidence);

    REQUIRE(
        single.confidence ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence::
            definitive);

    /*
     * Collapsed-core evidence receives high confidence.
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
            result.summary);

    REQUIRE(collapsed.total_traces == 8U);
    REQUIRE(collapsed.confidence_basis_points == 7500U);
    REQUIRE(collapsed.high_confidence);
    REQUIRE(!collapsed.sample_limited);

    /*
     * Exact outlier-dominance threshold evidence is high
     * confidence but boundary-adjacent.
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
            result.summary);

    REQUIRE(
        outlier.outlier_trace_coverage_basis_points ==
        2500U);

    REQUIRE(outlier.confidence_basis_points == 7500U);
    REQUIRE(outlier.evidence_margin_basis_points == 0U);
    REQUIRE(outlier.boundary_adjacent);
    REQUIRE(outlier.high_confidence);

    /*
     * Maximal skew receives definitive confidence.
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
            result.summary);

    REQUIRE(right.absolute_skewness_basis_points == 10000U);
    REQUIRE(right.evidence_margin_basis_points == 9000U);
    REQUIRE(right.confidence_basis_points == 10000U);
    REQUIRE(right.definitive_confidence);

    /*
     * Compact symmetry exactly on its governing boundary
     * receives moderate confidence.
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
            result.summary);

    REQUIRE(compact.total_traces == 8U);
    REQUIRE(compact.total_status_range == 40U);
    REQUIRE(compact.interquartile_status_range == 20U);

    REQUIRE(compact.evidence_margin_basis_points == 0U);
    REQUIRE(compact.confidence_basis_points == 5000U);
    REQUIRE(compact.boundary_adjacent);
    REQUIRE(compact.moderate_confidence);

    /*
     * Five-trace dispersed evidence would otherwise be
     * strong, but the small matched set caps confidence.
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
            result.summary);

    REQUIRE(dispersed.total_traces == 5U);
    REQUIRE(dispersed.total_status_range == 20U);
    REQUIRE(dispersed.interquartile_status_range == 18U);

    REQUIRE(dispersed.evidence_margin_basis_points == 8000U);
    REQUIRE(dispersed.confidence_basis_points == 5000U);
    REQUIRE(dispersed.sample_limited);
    REQUIRE(dispersed.moderate_confidence);

    /*
     * Filtering, pagination, and metadata-only shaping
     * preserve matched-set confidence.
     */

    RuntimeServiceCapabilityTraceQuery filtered_query;
    filtered_query.capability = "right";

    const auto filtered_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            filtered_query);

    const auto filtered =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
            page_result.summary);

    REQUIRE(page_result.matched_count == 8U);
    REQUIRE(page_result.returned_count == 2U);

    REQUIRE(
        page.confidence_basis_points ==
        filtered.confidence_basis_points);

    REQUIRE(
        page.evidence_margin_basis_points ==
        filtered.evidence_margin_basis_points);

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.capability = "right";
    metadata_query.maximum_results = 0U;

    const auto metadata_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    const auto metadata =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
            metadata_result.summary);

    REQUIRE(metadata_result.matched_count == 8U);
    REQUIRE(metadata_result.returned_count == 0U);
    REQUIRE(metadata_result.traces.empty());

    REQUIRE(
        metadata.confidence_basis_points ==
        filtered.confidence_basis_points);

    REQUIRE(metadata.definitive_confidence);

    /*
     * No-match queries restore undefined confidence.
     */

    RuntimeServiceCapabilityTraceQuery missing_query;
    missing_query.capability = "missing";

    const auto missing_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            missing_query);

    const auto missing =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
            missing_result.summary);

    REQUIRE(missing.total_traces == 0U);
    REQUIRE(!missing.confidence_defined);
    REQUIRE(missing.confidence_basis_points == 0U);

    REQUIRE(
        missing.confidence ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence::
            undefined);

    return 0;
}
