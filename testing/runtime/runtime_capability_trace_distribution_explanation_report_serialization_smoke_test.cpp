#include "runtime_service_capability_trace_query.hpp"

#include <cstdint>
#include <deque>
#include <initializer_list>
#include <string>
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


bool Contains(
    const std::string& value,
    const char* expected)
{
    return value.find(expected) != std::string::npos;
}

} // namespace


int main()
{
    using namespace sovereign;

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        traces;

    /*
     * Empty matched sets serialize deterministically because
     * their explanation is structurally conclusive.
     */

    auto result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    const auto empty =
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            result.summary);

    REQUIRE(empty.schema_version == 1U);
    REQUIRE(empty.serialization_available);
    REQUIRE(empty.matched_set_stable);

    REQUIRE(
        Contains(
            empty.payload,
            "\"shape\":\"empty\""));

    REQUIRE(
        Contains(
            empty.payload,
            "\"governing_rule\":\"empty_distribution\""));

    REQUIRE(
        Contains(
            empty.payload,
            "\"stability\":\"stable\""));

    std::uint64_t sequence = 1U;

    AddDistribution(
        traces,
        sequence,
        "right",
        {0U, 0U, 0U, 0U, 0U, 10U, 20U, 30U});

    RuntimeServiceCapabilityTraceQuery query;
    query.capability = "right";

    result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            query);

    const auto right =
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            result.summary);

    REQUIRE(right.serialization_available);
    REQUIRE(right.matched_set_stable);

    REQUIRE(
        Contains(
            right.payload,
            "\"shape\":\"right_skewed\""));

    REQUIRE(
        Contains(
            right.payload,
            "\"governing_rule\":\"right_quartile_skew\""));

    REQUIRE(
        Contains(
            right.payload,
            "\"confidence\":\"definitive\""));

    REQUIRE(
        Contains(
            right.payload,
            "\"stability\":\"stable\""));

    REQUIRE(
        Contains(
            right.payload,
            "\"stability_basis_points\":10000"));

    /*
     * Repeated serialization is byte-for-byte deterministic.
     */

    const auto repeated =
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            result.summary);

    REQUIRE(repeated.payload == right.payload);

    /*
     * Pagination and metadata-only shaping preserve the
     * complete matched-set serialized report.
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
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            page_result.summary);

    REQUIRE(page_result.matched_count == 8U);
    REQUIRE(page_result.returned_count == 2U);
    REQUIRE(page.payload == right.payload);

    RuntimeServiceCapabilityTraceQuery metadata_query;
    metadata_query.capability = "right";
    metadata_query.maximum_results = 0U;

    const auto metadata_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    const auto metadata =
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            metadata_result.summary);

    REQUIRE(metadata_result.matched_count == 8U);
    REQUIRE(metadata_result.returned_count == 0U);
    REQUIRE(metadata.payload == right.payload);

    return 0;
}
