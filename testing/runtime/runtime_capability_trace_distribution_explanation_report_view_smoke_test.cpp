#include "runtime_service_capability_trace_query.hpp"

#include <cstdint>
#include <deque>
#include "test_require.hpp"

namespace
{

sovereign::RuntimeServiceCapabilityPipelineTrace
MakeTrace(
    const std::uint64_t sequence,
    const std::uint8_t status)
{
    sovereign::RuntimeServiceCapabilityPipelineTrace trace;

    trace.invocation_sequence = sequence;
    trace.capability = "report";
    trace.operation = "execute";
    trace.selected_service_name = "provider";
    trace.result_status = status;

    return trace;
}

}

int main()
{
    using namespace sovereign;

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        traces;

    auto empty =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto empty_report =
        BuildRuntimeServiceCapabilityTraceDistributionExplanationReportView(
            empty.summary);

    REQUIRE(empty_report.report_defined);
    REQUIRE(empty_report.stable);
    REQUIRE(empty_report.structurally_anchored);
    REQUIRE(empty_report.stability_basis_points == 10000U);
    REQUIRE(empty_report.matched_set_stable);

    std::uint64_t sequence = 1U;

    for (const auto status :
         {0U,0U,0U,0U,0U,10U,20U,30U})
    {
        traces.push_back(
            MakeTrace(
                sequence++,
                static_cast<std::uint8_t>(status)));
    }

    auto result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto report =
        BuildRuntimeServiceCapabilityTraceDistributionExplanationReportView(
            result.summary);

    REQUIRE(report.report_defined);
    REQUIRE(report.stable);
    REQUIRE(report.stability_basis_points == 10000U);

    return 0;
}
