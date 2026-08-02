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

    using Stability =
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability;

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        traces;

    /*
     * Empty distributions remain structurally anchored.
     */

    auto result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    auto stability =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability(
            result.summary);

    REQUIRE(stability.stability_defined);
    REQUIRE(stability.matched_set_stable);
    REQUIRE(stability.structurally_anchored);
    REQUIRE(stability.stable);
    REQUIRE(!stability.unstable);
    REQUIRE(stability.stability_basis_points == 10000U);
    REQUIRE(stability.stability == Stability::stable);

    std::uint64_t sequence = 1U;

    /*
     * Zero complete spread is structurally stable.
     */

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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability(
            result.summary);

    REQUIRE(single.structurally_anchored);
    REQUIRE(single.stable);
    REQUIRE(single.stability == Stability::stable);
    REQUIRE(single.stability_basis_points == 10000U);

    /*
     * Exact outlier-threshold evidence is unstable.
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability(
            result.summary);

    REQUIRE(outlier.threshold_sensitive);
    REQUIRE(outlier.unstable);
    REQUIRE(!outlier.stable);
    REQUIRE(outlier.stability == Stability::unstable);
    REQUIRE(outlier.stability_basis_points == 0U);

    /*
     * Exact symmetry-boundary evidence is unstable.
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability(
            result.summary);

    REQUIRE(compact.threshold_sensitive);
    REQUIRE(compact.unstable);
    REQUIRE(compact.stability == Stability::unstable);

    /*
     * A small but otherwise coherent sample is conditional.
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability(
            result.summary);

    REQUIRE(dispersed.sample_sensitive);
    REQUIRE(dispersed.conditionally_stable);
    REQUIRE(!dispersed.stable);
    REQUIRE(!dispersed.unstable);

    REQUIRE(
        dispersed.stability ==
        Stability::conditionally_stable);

    REQUIRE(
        dispersed.stability_basis_points ==
        dispersed.confidence_basis_points);

    /*
     * Strong directional evidence is stable.
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
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability(
            result.summary);

    REQUIRE(right.stable);
    REQUIRE(!right.threshold_sensitive);
    REQUIRE(!right.evidence_sensitive);
    REQUIRE(!right.sample_sensitive);
    REQUIRE(right.stability == Stability::stable);
    REQUIRE(right.stability_basis_points == 10000U);

    /*
     * Pagination preserves matched-set stability diagnostics.
     */

    RuntimeServiceCapabilityTraceQuery page_query;
    page_query.capability = "right";
    page_query.offset = 2U;
    page_query.maximum_results = 2U;

    const auto page_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            page_query);

    const auto page =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability(
            page_result.summary);

    REQUIRE(page_result.matched_count == 8U);
    REQUIRE(page_result.returned_count == 2U);

    REQUIRE(page.shape == right.shape);
    REQUIRE(page.governing_rule == right.governing_rule);
    REQUIRE(page.confidence == right.confidence);
    REQUIRE(page.stability == right.stability);

    REQUIRE(
        page.stability_basis_points ==
        right.stability_basis_points);

    REQUIRE(page.matched_set_stable);

    /*
     * Metadata-only queries preserve the same result.
     */

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.capability = "right";
    metadata_query.maximum_results = 0U;

    const auto metadata_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    const auto metadata =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability(
            metadata_result.summary);

    REQUIRE(metadata_result.matched_count == 8U);
    REQUIRE(metadata_result.returned_count == 0U);

    REQUIRE(metadata.shape == right.shape);
    REQUIRE(metadata.governing_rule == right.governing_rule);
    REQUIRE(metadata.confidence == right.confidence);
    REQUIRE(metadata.stability == right.stability);

    REQUIRE(
        metadata.stability_basis_points ==
        right.stability_basis_points);

    REQUIRE(metadata.matched_set_stable);

    return 0;
}
