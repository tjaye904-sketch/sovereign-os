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
    sovereign::RuntimeServiceCapabilityPipelineTrace trace;

    trace.invocation_sequence = sequence;
    trace.capability = capability;
    trace.operation = "execute";
    trace.selected_service_name = "provider";
    trace.result_status = result_status;

    return trace;
}

}

int main()
{
    using namespace sovereign;

    std::deque<RuntimeServiceCapabilityPipelineTrace> traces;

    auto result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    auto empty =
        BuildRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStabilityReport(
            result.summary);

    REQUIRE(empty.report_available);
    REQUIRE(empty.explanation_reliable);
    REQUIRE(empty.stability_reliable);

    traces.push_back(
        MakeTrace(1U, "stable", 0U));
    traces.push_back(
        MakeTrace(2U, "stable", 0U));
    traces.push_back(
        MakeTrace(3U, "stable", 0U));
    traces.push_back(
        MakeTrace(4U, "stable", 10U));
    traces.push_back(
        MakeTrace(5U, "stable", 20U));
    traces.push_back(
        MakeTrace(6U, "stable", 30U));

    RuntimeServiceCapabilityTraceQuery query;
    query.capability = "stable";

    result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            query);

    const auto stable =
        BuildRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStabilityReport(
            result.summary);

    REQUIRE(stable.report_available);
    REQUIRE(stable.explanation_reliable);
    REQUIRE(stable.stability_reliable);
    REQUIRE(stable.stability.stability_defined);
    REQUIRE(stable.overall_basis_points ==
        stable.stability.stability_basis_points);

    RuntimeServiceCapabilityTraceQuery page_query;
    page_query.capability = "stable";
    page_query.offset = 2U;
    page_query.maximum_results = 2U;

    const auto page_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            page_query);

    const auto page =
        BuildRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStabilityReport(
            page_result.summary);

    REQUIRE(page.explanation.shape ==
        stable.explanation.shape);

    REQUIRE(page.stability.stability ==
        stable.stability.stability);

    return 0;
}
