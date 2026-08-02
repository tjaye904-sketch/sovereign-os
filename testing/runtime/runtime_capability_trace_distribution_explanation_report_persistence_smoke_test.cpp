#include "runtime_service_capability_trace_report_persistence.hpp"

#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
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


void WritePayload(
    const std::filesystem::path& path,
    const std::string& payload)
{
    std::ofstream stream(
        path,
        std::ios::binary |
            std::ios::trunc);

    REQUIRE(stream.is_open());

    stream.write(
        payload.data(),
        static_cast<std::streamsize>(
            payload.size()));

    REQUIRE(stream.good());
}

} // namespace


int main()
{
    using namespace sovereign;

    using PersistenceStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceStatus;

    using LoadStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportLoadStatus;

    const auto root =
        std::filesystem::temp_directory_path() /
        "sovereign_rt144_persistence_smoke_test";

    std::error_code cleanup_error;

    std::filesystem::remove_all(
        root,
        cleanup_error);

    const auto report_path =
        root /
        "nested" /
        "report.json";

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

    /*
     * Persistence creates missing parent directories and
     * stores the canonical payload.
     */

    const auto persisted =
        PersistRuntimeServiceCapabilityTraceDistributionExplanationReport(
            query_result.summary,
            report_path);

    REQUIRE(persisted.persisted);
    REQUIRE(
        persisted.status ==
        PersistenceStatus::persisted);

    REQUIRE(
        persisted.bytes_written ==
        serialized.payload.size());

    REQUIRE(
        std::filesystem::exists(
            report_path));

    REQUIRE(
        !std::filesystem::exists(
            std::filesystem::path(
                report_path.string() + ".tmp")));

    const auto loaded =
        LoadRuntimeServiceCapabilityTraceDistributionExplanationReport(
            report_path);

    REQUIRE(loaded.loaded);
    REQUIRE(
        loaded.status ==
        LoadStatus::loaded);

    REQUIRE(
        loaded.bytes_read ==
        serialized.payload.size());

    REQUIRE(
        loaded.deserialization.report.shape ==
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            right_skewed);

    const auto loaded_serialization =
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportView(
            loaded.deserialization.report);

    REQUIRE(
        loaded_serialization.payload ==
        serialized.payload);

    /*
     * Re-persistence deterministically replaces the existing
     * destination without leaving a temporary file.
     */

    const auto repeated =
        PersistRuntimeServiceCapabilityTraceDistributionExplanationReport(
            query_result.summary,
            report_path);

    REQUIRE(repeated.persisted);
    REQUIRE(
        repeated.bytes_written ==
        serialized.payload.size());

    const auto repeated_load =
        LoadRuntimeServiceCapabilityTraceDistributionExplanationReport(
            report_path);

    REQUIRE(repeated_load.loaded);

    const auto repeated_serialization =
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportView(
            repeated_load.deserialization.report);

    REQUIRE(
        repeated_serialization.payload ==
        serialized.payload);

    /*
     * Missing, malformed, and oversized files are classified.
     */

    const auto missing =
        LoadRuntimeServiceCapabilityTraceDistributionExplanationReport(
            root / "missing.json");

    REQUIRE(!missing.loaded);
    REQUIRE(
        missing.status ==
        LoadStatus::file_not_found);

    const auto malformed_path =
        root /
        "malformed.json";

    WritePayload(
        malformed_path,
        "{\"schema_version\":1}");

    const auto malformed =
        LoadRuntimeServiceCapabilityTraceDistributionExplanationReport(
            malformed_path);

    REQUIRE(!malformed.loaded);
    REQUIRE(
        malformed.status ==
        LoadStatus::validation_failed);

    REQUIRE(
        malformed.deserialization.validation_status ==
        RuntimeServiceCapabilityTraceDistributionExplanationReportValidationStatus::
            malformed_payload);

    RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceOptions
        constrained_options;

    constrained_options.maximum_payload_bytes = 8U;

    const auto oversized =
        LoadRuntimeServiceCapabilityTraceDistributionExplanationReport(
            report_path,
            constrained_options);

    REQUIRE(!oversized.loaded);
    REQUIRE(
        oversized.status ==
        LoadStatus::payload_too_large);

    /*
     * An unavailable serialization cannot be persisted.
     */

    RuntimeServiceCapabilityTraceDistributionExplanationReportSerialization
        unavailable;

    const auto unavailable_result =
        PersistRuntimeServiceCapabilityTraceDistributionExplanationReportSerialization(
            unavailable,
            root / "unavailable.json");

    REQUIRE(!unavailable_result.persisted);
    REQUIRE(
        unavailable_result.status ==
        PersistenceStatus::serialization_unavailable);

    std::filesystem::remove_all(
        root,
        cleanup_error);

    REQUIRE(!cleanup_error);

    return 0;
}
