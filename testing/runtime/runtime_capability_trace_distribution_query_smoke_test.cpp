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
    const char* capability,
    const bool completed,
    const std::uint8_t result_status)
{
    sovereign::RuntimeServiceCapabilityPipelineTrace trace;

    trace.invocation_sequence = sequence;
    trace.capability = capability;
    trace.operation = "execute";
    trace.selected_service_name = "service-a";

    trace.request_policy_evaluated = true;
    trace.negotiation_attempted = true;
    trace.provider_policy_evaluated = true;
    trace.invocation_attempted = true;

    trace.completed = completed;
    trace.result_status = result_status;

    return trace;
}

} // namespace

int main()
{
    using namespace sovereign;

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        traces;

    traces.push_back(
        MakeTrace(1U, "alpha", true, 4U));

    traces.push_back(
        MakeTrace(2U, "beta", true, 0U));

    traces.push_back(
        MakeTrace(3U, "alpha", false, 255U));

    traces.push_back(
        MakeTrace(4U, "alpha", true, 4U));

    traces.push_back(
        MakeTrace(5U, "beta", false, 17U));

    const auto unfiltered =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    REQUIRE(unfiltered.matched_count == 5U);
    REQUIRE(unfiltered.returned_count == 5U);

    const auto distribution =
        unfiltered.summary
            .ResultStatusDistribution();

    REQUIRE(distribution.size() == 4U);

    REQUIRE(distribution[0U].result_status == 0U);
    REQUIRE(distribution[0U].count == 1U);

    REQUIRE(distribution[1U].result_status == 4U);
    REQUIRE(distribution[1U].count == 2U);

    REQUIRE(distribution[2U].result_status == 17U);
    REQUIRE(distribution[2U].count == 1U);

    REQUIRE(distribution[3U].result_status == 255U);
    REQUIRE(distribution[3U].count == 1U);

    for (const auto& entry : distribution)
    {
        REQUIRE(entry.count != 0U);

        REQUIRE(
            entry.count ==
            unfiltered.summary.ResultStatusCount(
                entry.result_status));
    }

    RuntimeServiceCapabilityTraceQuery alpha_query;
    alpha_query.capability = "alpha";

    const auto alpha =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            alpha_query);

    REQUIRE(alpha.matched_count == 3U);

    const auto alpha_distribution =
        alpha.summary.ResultStatusDistribution();

    REQUIRE(alpha_distribution.size() == 2U);

    REQUIRE(
        alpha_distribution[0U].result_status ==
        4U);

    REQUIRE(
        alpha_distribution[0U].count ==
        2U);

    REQUIRE(
        alpha_distribution[1U].result_status ==
        255U);

    REQUIRE(
        alpha_distribution[1U].count ==
        1U);

    RuntimeServiceCapabilityTraceQuery status_query;
    status_query.result_status = 4U;

    const auto status_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            status_query);

    const auto status_distribution =
        status_result.summary
            .ResultStatusDistribution();

    REQUIRE(status_result.matched_count == 2U);
    REQUIRE(status_distribution.size() == 1U);

    REQUIRE(
        status_distribution.front().result_status ==
        4U);

    REQUIRE(
        status_distribution.front().count ==
        2U);

    RuntimeServiceCapabilityTraceQuery page_query;

    page_query.sort_order =
        RuntimeServiceCapabilityTraceSortOrder::
            NewestFirst;

    page_query.offset = 1U;
    page_query.maximum_results = 1U;

    const auto page =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            page_query);

    REQUIRE(page.matched_count == 5U);
    REQUIRE(page.returned_count == 1U);
    REQUIRE(page.has_more);

    const auto page_distribution =
        page.summary.ResultStatusDistribution();

    REQUIRE(
        page_distribution.size() ==
        distribution.size());

    for (
        std::size_t index = 0U;
        index < distribution.size();
        ++index)
    {
        REQUIRE(
            page_distribution[index].result_status ==
            distribution[index].result_status);

        REQUIRE(
            page_distribution[index].count ==
            distribution[index].count);
    }

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.maximum_results = 0U;

    const auto metadata =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    REQUIRE(metadata.matched_count == 5U);
    REQUIRE(metadata.returned_count == 0U);
    REQUIRE(metadata.traces.empty());

    REQUIRE(
        metadata.execution_diagnostics.metadata_only);

    REQUIRE(
        metadata.summary
            .ResultStatusDistribution()
            .size() ==
        distribution.size());

    RuntimeServiceCapabilityTraceQuery no_match_query;
    no_match_query.capability = "missing";

    const auto no_matches =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            no_match_query);

    REQUIRE(no_matches.matched_count == 0U);
    REQUIRE(no_matches.returned_count == 0U);

    REQUIRE(
        no_matches.summary
            .ResultStatusDistribution()
            .empty());

    return 0;
}
