#include "runtime_service_capability_trace_report_deserialization.hpp"

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
        sovereign::RuntimeServiceCapabilityPipelineTrace>& traces,
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


std::string ReplaceOnce(
    std::string value,
    const std::string& from,
    const std::string& to)
{
    const auto position =
        value.find(from);

    REQUIRE(position != std::string::npos);

    value.replace(
        position,
        from.size(),
        to);

    return value;
}

} // namespace


int main()
{
    using namespace sovereign;

    using ValidationStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportValidationStatus;

    const auto empty_payload =
        DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            "");

    REQUIRE(!empty_payload.deserialization_available);
    REQUIRE(
        empty_payload.validation_status ==
        ValidationStatus::empty_payload);

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        traces;

    std::uint64_t sequence = 1U;

    AddDistribution(
        traces,
        sequence,
        "right",
        {0U, 0U, 0U, 0U, 0U, 10U, 20U, 30U});

    RuntimeServiceCapabilityTraceQuery query;
    query.capability = "right";

    const auto query_result =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            query);

    const auto serialized =
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            query_result.summary);

    REQUIRE(serialized.serialization_available);

    const auto deserialized =
        DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            serialized.payload);

    REQUIRE(deserialized.deserialization_available);
    REQUIRE(
        deserialized.validation_status ==
        ValidationStatus::valid);

    REQUIRE(deserialized.report.report_defined);
    REQUIRE(deserialized.report.total_traces == 8U);
    REQUIRE(deserialized.report.represented_statuses == 4U);

    REQUIRE(
        deserialized.report.shape ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            right_skewed);

    REQUIRE(
        deserialized.report.governing_rule ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
            right_quartile_skew);

    REQUIRE(
        deserialized.report.confidence ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence::
            definitive);

    REQUIRE(
        deserialized.report.stability ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability::
            stable);

    REQUIRE(deserialized.report.stable);
    REQUIRE(!deserialized.report.conditionally_stable);
    REQUIRE(!deserialized.report.unstable);
    REQUIRE(deserialized.report.matched_set_stable);

    /*
     * Canonical round trip must be byte-for-byte stable.
     */

    const auto round_trip =
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportView(
            deserialized.report);

    REQUIRE(round_trip.serialization_available);
    REQUIRE(round_trip.payload == serialized.payload);

    /*
     * Validation failures must be classified.
     */

    const auto unsupported_version =
        DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            ReplaceOnce(
                serialized.payload,
                "\"schema_version\":1",
                "\"schema_version\":2"));

    REQUIRE(!unsupported_version.deserialization_available);
    REQUIRE(
        unsupported_version.validation_status ==
        ValidationStatus::unsupported_schema_version);

    const auto invalid_shape =
        DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            ReplaceOnce(
                serialized.payload,
                "\"shape\":\"right_skewed\"",
                "\"shape\":\"not_a_shape\""));

    REQUIRE(!invalid_shape.deserialization_available);
    REQUIRE(
        invalid_shape.validation_status ==
        ValidationStatus::invalid_shape);

    const auto invalid_boolean =
        DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            ReplaceOnce(
                serialized.payload,
                "\"matched_set_stable\":true",
                "\"matched_set_stable\":maybe"));

    REQUIRE(!invalid_boolean.deserialization_available);
    REQUIRE(
        invalid_boolean.validation_status ==
        ValidationStatus::invalid_boolean);

    const auto out_of_range =
        DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            ReplaceOnce(
                serialized.payload,
                "\"stability_basis_points\":10000",
                "\"stability_basis_points\":10001"));

    REQUIRE(!out_of_range.deserialization_available);
    REQUIRE(
        out_of_range.validation_status ==
        ValidationStatus::numeric_value_out_of_range);

    const auto inconsistent =
        DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            ReplaceOnce(
                serialized.payload,
                "\"stable\":true",
                "\"stable\":false"));

    REQUIRE(!inconsistent.deserialization_available);
    REQUIRE(
        inconsistent.validation_status ==
        ValidationStatus::inconsistent_report);

    const auto trailing_data =
        DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            serialized.payload + " trailing");

    REQUIRE(!trailing_data.deserialization_available);
    REQUIRE(
        trailing_data.validation_status ==
        ValidationStatus::malformed_payload);

    return 0;
}
