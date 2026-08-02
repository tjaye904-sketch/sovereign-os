#include "runtime_service_capability_trace_query.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include "test_require.hpp"

namespace
{

sovereign::RuntimeServiceCapabilityPipelineTrace
MakeTrace(
    const std::uint64_t sequence,
    const char* capability)
{
    sovereign::RuntimeServiceCapabilityPipelineTrace
        trace;

    trace.invocation_sequence = sequence;
    trace.capability = capability;
    trace.operation = "execute";
    trace.selected_service_name = "provider";

    return trace;
}

std::size_t DistributionTotal(
    const sovereign::
        RuntimeServiceCapabilityTraceQuerySummary&
            summary)
{
    std::size_t total = 0U;

    for (
        const auto& entry :
        summary.ResultStatusDistribution())
    {
        const auto& [result_status, trace_count] =
            entry;

        static_cast<void>(result_status);

        total += trace_count;
    }

    return total;
}

} // namespace

int main()
{
    using namespace sovereign;

    /*
     * Empty query results produce deterministic empty
     * distribution diagnostics.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        empty_traces;

    const auto empty_result =
        QueryRuntimeServiceCapabilityTraces(
            empty_traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto empty_diagnostics =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistribution(
            empty_result.summary);

    REQUIRE(empty_diagnostics.total_traces == 0U);
    REQUIRE(
        empty_diagnostics.represented_statuses ==
        0U);
    REQUIRE(
        empty_diagnostics.minimum_status_count ==
        0U);
    REQUIRE(
        empty_diagnostics.maximum_status_count ==
        0U);
    REQUIRE(empty_diagnostics.empty);
    REQUIRE(empty_diagnostics.uniform);

    /*
     * Multiple traces with the same result status produce
     * a single represented status with a uniform
     * distribution.
     */

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        traces;

    traces.push_back(
        MakeTrace(
            1U,
            "alpha"));

    traces.push_back(
        MakeTrace(
            2U,
            "beta"));

    traces.push_back(
        MakeTrace(
            3U,
            "alpha"));

    const auto all =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto all_diagnostics =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistribution(
            all.summary);

    REQUIRE(all.matched_count == 3U);
    REQUIRE(all.returned_count == 3U);

    REQUIRE(
        all_diagnostics.total_traces ==
        DistributionTotal(all.summary));

    REQUIRE(all_diagnostics.total_traces == 3U);
    REQUIRE(
        all_diagnostics.represented_statuses ==
        all.summary
            .ResultStatusDistribution()
            .size());

    REQUIRE(!all_diagnostics.empty);

    REQUIRE(
        all_diagnostics.minimum_status_count <=
        all_diagnostics.maximum_status_count);

    /*
     * Pagination shapes only the returned page. Distribution
     * diagnostics continue to describe the complete matched
     * result set.
     */

    RuntimeServiceCapabilityTraceQuery page_query;

    page_query.offset = 1U;
    page_query.maximum_results = 1U;

    const auto page =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            page_query);

    const auto page_diagnostics =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistribution(
            page.summary);

    REQUIRE(page.matched_count == 3U);
    REQUIRE(page.returned_count == 1U);
    REQUIRE(page.has_more);

    REQUIRE(
        page_diagnostics.total_traces ==
        all_diagnostics.total_traces);

    REQUIRE(
        page_diagnostics.represented_statuses ==
        all_diagnostics.represented_statuses);

    REQUIRE(
        page_diagnostics.minimum_status_count ==
        all_diagnostics.minimum_status_count);

    REQUIRE(
        page_diagnostics.maximum_status_count ==
        all_diagnostics.maximum_status_count);

    REQUIRE(
        page_diagnostics.empty ==
        all_diagnostics.empty);

    REQUIRE(
        page_diagnostics.uniform ==
        all_diagnostics.uniform);

    /*
     * Zero-limit metadata queries retain full distribution
     * diagnostics while returning no trace records.
     */

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.maximum_results = 0U;

    const auto metadata =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    const auto metadata_diagnostics =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistribution(
            metadata.summary);

    REQUIRE(metadata.matched_count == 3U);
    REQUIRE(metadata.returned_count == 0U);
    REQUIRE(metadata.traces.empty());

    REQUIRE(
        metadata.execution_diagnostics
            .metadata_only);

    REQUIRE(
        metadata_diagnostics.total_traces ==
        all_diagnostics.total_traces);

    REQUIRE(
        metadata_diagnostics.represented_statuses ==
        all_diagnostics.represented_statuses);

    /*
     * Filtering limits the diagnostics to matching traces.
     */

    RuntimeServiceCapabilityTraceQuery alpha_query;
    alpha_query.capability = "alpha";

    const auto alpha =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            alpha_query);

    const auto alpha_diagnostics =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistribution(
            alpha.summary);

    REQUIRE(alpha.matched_count == 2U);
    REQUIRE(alpha.returned_count == 2U);

    REQUIRE(
        alpha_diagnostics.total_traces ==
        2U);

    REQUIRE(
        alpha_diagnostics.total_traces ==
        DistributionTotal(alpha.summary));

    /*
     * A no-match query restores the deterministic empty
     * diagnostics contract.
     */

    RuntimeServiceCapabilityTraceQuery missing_query;
    missing_query.capability = "missing";

    const auto missing =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            missing_query);

    const auto missing_diagnostics =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistribution(
            missing.summary);

    REQUIRE(missing.matched_count == 0U);
    REQUIRE(missing.returned_count == 0U);

    REQUIRE(missing_diagnostics.total_traces == 0U);
    REQUIRE(
        missing_diagnostics.represented_statuses ==
        0U);
    REQUIRE(missing_diagnostics.empty);
    REQUIRE(missing_diagnostics.uniform);

    return 0;
}
