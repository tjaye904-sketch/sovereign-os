#pragma once

#include "runtime_service_capability_trace_report_persistence.hpp"
#include "runtime_service_capability_trace_report_schema_version_detection.hpp"

#include <string>
#include <string_view>

namespace sovereign
{

enum class RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStatus
{
    migrated,
    already_current,
    unsupported_source_version,
    malformed_payload,
    migration_failed
};

struct RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationResult
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStatus
        status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStatus::
                malformed_payload};

    std::string migrated_payload{};

    std::uint32_t source_version{0};
    std::uint32_t target_version{0};

    bool migrated{false};
};

/*
 * RT-145 foundation:
 *
 * The runtime now understands versioned documents.
 *
 * Since schema_version == 1 is the current format,
 * migration is intentionally idempotent.
 *
 * Future milestones (RT-146+) extend this dispatcher
 * without changing callers.
 */

inline RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationResult
MigrateRuntimeServiceCapabilityTraceDistributionExplanationReport(
    const std::string_view payload)
{
    using Status =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStatus;

    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationResult
        result;

    const auto deserialized =
        DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            payload);

    if (!deserialized.deserialization_available)
    {
        result.status =
            Status::malformed_payload;

        return result;
    }

    result.source_version =
        RuntimeServiceCapabilityTraceDistributionExplanationCurrentSchemaVersion;

    result.target_version =
        RuntimeServiceCapabilityTraceDistributionExplanationCurrentSchemaVersion;

    const auto canonical =
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportView(
            deserialized.report);

    if (!canonical.serialization_available)
    {
        result.status =
            Status::migration_failed;

        return result;
    }

    result.status =
        Status::already_current;

    result.migrated_payload =
        canonical.payload;

    result.migrated = true;

    return result;
}

}
