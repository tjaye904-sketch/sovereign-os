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
     * diagnostics and no classification flags.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        empty_traces;

    const auto empty_result =
        QueryRuntimeServiceCapabilityTraces(
            empty_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto empty =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionStatistics(
            empty_result.summary);

    REQUIRE(empty.total_traces == 0U);
    REQUIRE(empty.represented_statuses == 0U);
    REQUIRE(empty.status_range_width == 0U);

    REQUIRE(
        empty.unrepresented_statuses_within_range ==
        0U);

    REQUIRE(
        empty.status_range_coverage_basis_points ==
        0U);

    REQUIRE(empty.dominant_trace_count == 0U);
    REQUIRE(empty.dominant_statuses == 0U);
    REQUIRE(empty.empty);
    REQUIRE(!empty.contiguous_status_range);
    REQUIRE(!empty.tied_dominance);
    REQUIRE(!empty.majority_dominance);
    REQUIRE(!empty.sparse_status_range);

    /*
     * A single represented status occupies its complete
     * one-status range and has strict majority dominance.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        single_status_traces;

    single_status_traces.push_back(
        MakeTrace(1U, "single", 4U));

    single_status_traces.push_back(
        MakeTrace(2U, "single", 4U));

    const auto single_result =
        QueryRuntimeServiceCapabilityTraces(
            single_status_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto single =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionStatistics(
            single_result.summary);

    REQUIRE(single.total_traces == 2U);
    REQUIRE(single.represented_statuses == 1U);
    REQUIRE(single.status_range_width == 1U);

    REQUIRE(
        single.unrepresented_statuses_within_range ==
        0U);

    REQUIRE(
        single.status_range_coverage_basis_points ==
        10000U);

    REQUIRE(single.dominant_trace_count == 2U);
    REQUIRE(single.dominant_statuses == 1U);
    REQUIRE(!single.empty);
    REQUIRE(single.contiguous_status_range);
    REQUIRE(!single.tied_dominance);
    REQUIRE(single.majority_dominance);
    REQUIRE(!single.sparse_status_range);

    /*
     * Consecutive represented statuses provide complete
     * status-range coverage.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        contiguous_traces;

    contiguous_traces.push_back(
        MakeTrace(1U, "contiguous", 7U));

    contiguous_traces.push_back(
        MakeTrace(2U, "contiguous", 8U));

    contiguous_traces.push_back(
        MakeTrace(3U, "contiguous", 9U));

    const auto contiguous_result =
        QueryRuntimeServiceCapabilityTraces(
            contiguous_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto contiguous =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionStatistics(
            contiguous_result.summary);

    REQUIRE(contiguous.total_traces == 3U);
    REQUIRE(contiguous.represented_statuses == 3U);
    REQUIRE(contiguous.status_range_width == 3U);

    REQUIRE(
        contiguous.unrepresented_statuses_within_range ==
        0U);

    REQUIRE(
        contiguous.status_range_coverage_basis_points ==
        10000U);

    REQUIRE(contiguous.contiguous_status_range);
    REQUIRE(contiguous.tied_dominance);
    REQUIRE(!contiguous.majority_dominance);
    REQUIRE(!contiguous.sparse_status_range);

    /*
     * A wide, lightly represented range is sparse. The full
     * uint8_t status domain remains supported.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        sparse_traces;

    sparse_traces.push_back(
        MakeTrace(1U, "alpha", 0U));

    sparse_traces.push_back(
        MakeTrace(2U, "alpha", 4U));

    sparse_traces.push_back(
        MakeTrace(3U, "alpha", 4U));

    sparse_traces.push_back(
        MakeTrace(4U, "beta", 17U));

    sparse_traces.push_back(
        MakeTrace(5U, "alpha", 255U));

    const auto sparse_result =
        QueryRuntimeServiceCapabilityTraces(
            sparse_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto sparse =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionStatistics(
            sparse_result.summary);

    REQUIRE(sparse.total_traces == 5U);
    REQUIRE(sparse.represented_statuses == 4U);
    REQUIRE(sparse.status_range_width == 256U);

    REQUIRE(
        sparse.unrepresented_statuses_within_range ==
        252U);

    REQUIRE(
        sparse.status_range_coverage_basis_points ==
        156U);

    REQUIRE(sparse.dominant_trace_count == 2U);
    REQUIRE(sparse.dominant_statuses == 1U);
    REQUIRE(!sparse.empty);
    REQUIRE(!sparse.contiguous_status_range);
    REQUIRE(!sparse.tied_dominance);
    REQUIRE(!sparse.majority_dominance);
    REQUIRE(sparse.sparse_status_range);

    /*
     * Filtering defines the statistical diagnostics domain.
     */

    RuntimeServiceCapabilityTraceQuery alpha_query;
    alpha_query.capability = "alpha";

    const auto alpha_result =
        QueryRuntimeServiceCapabilityTraces(
            sparse_traces,
            alpha_query);

    const auto alpha =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionStatistics(
            alpha_result.summary);

    REQUIRE(alpha_result.matched_count == 4U);
    REQUIRE(alpha.total_traces == 4U);
    REQUIRE(alpha.represented_statuses == 3U);
    REQUIRE(alpha.status_range_width == 256U);

    REQUIRE(
        alpha.unrepresented_statuses_within_range ==
        253U);

    REQUIRE(
        alpha.status_range_coverage_basis_points ==
        117U);

    REQUIRE(alpha.dominant_trace_count == 2U);
    REQUIRE(alpha.dominant_statuses == 1U);
    REQUIRE(!alpha.tied_dominance);
    REQUIRE(!alpha.majority_dominance);
    REQUIRE(alpha.sparse_status_range);

    /*
     * Strict majority requires more than half of all traces.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        majority_traces;

    majority_traces.push_back(
        MakeTrace(1U, "majority", 3U));

    majority_traces.push_back(
        MakeTrace(2U, "majority", 3U));

    majority_traces.push_back(
        MakeTrace(3U, "majority", 3U));

    majority_traces.push_back(
        MakeTrace(4U, "majority", 9U));

    const auto majority_result =
        QueryRuntimeServiceCapabilityTraces(
            majority_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto majority =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionStatistics(
            majority_result.summary);

    REQUIRE(majority.total_traces == 4U);
    REQUIRE(majority.dominant_trace_count == 3U);
    REQUIRE(majority.dominant_statuses == 1U);
    REQUIRE(majority.majority_dominance);
    REQUIRE(!majority.tied_dominance);

    /*
     * Pagination and metadata-only shaping preserve
     * diagnostics for the complete matched set.
     */

    RuntimeServiceCapabilityTraceQuery page_query;
    page_query.offset = 1U;
    page_query.maximum_results = 1U;

    const auto page_result =
        QueryRuntimeServiceCapabilityTraces(
            sparse_traces,
            page_query);

    const auto page =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionStatistics(
            page_result.summary);

    REQUIRE(page_result.matched_count == 5U);
    REQUIRE(page_result.returned_count == 1U);
    REQUIRE(page_result.has_more);

    REQUIRE(page.total_traces == sparse.total_traces);

    REQUIRE(
        page.represented_statuses ==
        sparse.represented_statuses);

    REQUIRE(
        page.status_range_width ==
        sparse.status_range_width);

    REQUIRE(
        page.status_range_coverage_basis_points ==
        sparse.status_range_coverage_basis_points);

    REQUIRE(
        page.dominant_trace_count ==
        sparse.dominant_trace_count);

    REQUIRE(
        page.sparse_status_range ==
        sparse.sparse_status_range);

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.maximum_results = 0U;

    const auto metadata_result =
        QueryRuntimeServiceCapabilityTraces(
            sparse_traces,
            metadata_query);

    const auto metadata =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionStatistics(
            metadata_result.summary);

    REQUIRE(metadata_result.matched_count == 5U);
    REQUIRE(metadata_result.returned_count == 0U);
    REQUIRE(metadata_result.traces.empty());

    REQUIRE(
        metadata_result.execution_diagnostics
            .metadata_only);

    REQUIRE(metadata.total_traces == sparse.total_traces);

    REQUIRE(
        metadata.status_range_coverage_basis_points ==
        sparse.status_range_coverage_basis_points);

    REQUIRE(
        metadata.majority_dominance ==
        sparse.majority_dominance);

    /*
     * A no-match query restores the empty diagnostics
     * contract.
     */

    RuntimeServiceCapabilityTraceQuery missing_query;
    missing_query.capability = "missing";

    const auto missing_result =
        QueryRuntimeServiceCapabilityTraces(
            sparse_traces,
            missing_query);

    const auto missing =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionStatistics(
            missing_result.summary);

    REQUIRE(missing_result.matched_count == 0U);
    REQUIRE(missing.total_traces == 0U);
    REQUIRE(missing.represented_statuses == 0U);
    REQUIRE(missing.empty);
    REQUIRE(!missing.sparse_status_range);

    return 0;
}
