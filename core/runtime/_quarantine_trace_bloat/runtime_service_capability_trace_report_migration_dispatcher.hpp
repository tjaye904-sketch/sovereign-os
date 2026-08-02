#pragma once

#include "runtime_service_capability_trace_report_migration.hpp"
#include "runtime_service_capability_trace_report_migration_registry.hpp"
#include "runtime_service_capability_trace_report_schema_version_detection.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace sovereign
{

enum class RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus
{
    current_schema_ready,
    current_schema_validation_failed,

    legacy_schema_unsupported,
    legacy_schema_migrated,
    legacy_migration_path_unavailable,
    legacy_migration_failed,
    legacy_migrated_payload_validation_failed,

    future_schema_unsupported,
    schema_version_missing,
    schema_version_malformed,
    migration_failed
};

struct RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchResult
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus
        status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus::
                schema_version_malformed};

    RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaProbe
        schema_probe{};

    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationResult
        migration{};

    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionResult
        registry_execution{};

    std::string canonical_payload{};
    std::string diagnostic{};

    std::uint32_t source_version{0};
    std::uint32_t target_version{
        RuntimeServiceCapabilityTraceDistributionExplanationCurrentSchemaVersion};

    bool registry_consulted{false};
    bool dispatch_succeeded{false};
    bool payload_available{false};
};

inline bool
FinalizeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatch(
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchResult&
        result,
    const std::string_view candidate_payload,
    const RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus
        success_status,
    const RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus
        validation_failure_status)
{
    using MigrationStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStatus;

    result.migration =
        MigrateRuntimeServiceCapabilityTraceDistributionExplanationReport(
            candidate_payload);

    if (result.migration.status ==
            MigrationStatus::already_current &&
        result.migration.migrated &&
        !result.migration.migrated_payload.empty())
    {
        result.status = success_status;
        result.canonical_payload =
            result.migration.migrated_payload;

        result.dispatch_succeeded = true;
        result.payload_available = true;

        return true;
    }

    if (result.migration.status ==
        MigrationStatus::malformed_payload)
    {
        result.status = validation_failure_status;
        return false;
    }

    result.status =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus::
            migration_failed;

    return false;
}

inline RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchResult
DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
    const std::string_view payload)
{
    using DispatchStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus;

    using SchemaStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus;

    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchResult
        result;

    result.schema_probe =
        ProbeRuntimeServiceCapabilityTraceDistributionExplanationReportSchemaVersion(
            payload);

    result.source_version =
        result.schema_probe.detected_version;

    switch (result.schema_probe.status)
    {
        case SchemaStatus::missing:
        {
            result.status =
                DispatchStatus::schema_version_missing;

            return result;
        }

        case SchemaStatus::malformed:
        {
            result.status =
                DispatchStatus::schema_version_malformed;

            return result;
        }

        case SchemaStatus::unsupported:
        {
            result.status =
                DispatchStatus::legacy_schema_unsupported;

            return result;
        }

        case SchemaStatus::future:
        {
            result.status =
                DispatchStatus::future_schema_unsupported;

            return result;
        }

        case SchemaStatus::current:
        {
            FinalizeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatch(
                result,
                payload,
                DispatchStatus::current_schema_ready,
                DispatchStatus::current_schema_validation_failed);

            return result;
        }
    }

    result.status =
        DispatchStatus::migration_failed;

    return result;
}

inline RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchResult
DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
    const std::string_view payload,
    const RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry&
        registry,
    const std::size_t maximum_migration_hops = 64U)
{
    using DispatchStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus;

    using SchemaStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus;

    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchResult
        result;

    result.schema_probe =
        ProbeRuntimeServiceCapabilityTraceDistributionExplanationReportSchemaVersion(
            payload);

    result.source_version =
        result.schema_probe.detected_version;

    switch (result.schema_probe.status)
    {
        case SchemaStatus::missing:
        {
            result.status =
                DispatchStatus::schema_version_missing;

            return result;
        }

        case SchemaStatus::malformed:
        {
            result.status =
                DispatchStatus::schema_version_malformed;

            return result;
        }

        case SchemaStatus::future:
        {
            result.status =
                DispatchStatus::future_schema_unsupported;

            return result;
        }

        case SchemaStatus::current:
        {
            FinalizeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatch(
                result,
                payload,
                DispatchStatus::current_schema_ready,
                DispatchStatus::current_schema_validation_failed);

            return result;
        }

        case SchemaStatus::unsupported:
        {
            result.registry_consulted = true;

            result.registry_execution =
                registry.Execute(
                    result.source_version,
                    result.target_version,
                    payload,
                    maximum_migration_hops);

            if (!result.registry_execution.migration_succeeded ||
                !result.registry_execution.payload_available)
            {
                using ExecutionStatus =
                    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionStatus;

                if (result.registry_execution.status ==
                    ExecutionStatus::path_resolution_failed)
                {
                    result.status =
                        DispatchStatus::legacy_migration_path_unavailable;
                }
                else
                {
                    result.status =
                        DispatchStatus::legacy_migration_failed;
                }

                result.diagnostic =
                    result.registry_execution.diagnostic;

                return result;
            }

            FinalizeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatch(
                result,
                result.registry_execution.migrated_payload,
                DispatchStatus::legacy_schema_migrated,
                DispatchStatus::legacy_migrated_payload_validation_failed);

            return result;
        }
    }

    result.status =
        DispatchStatus::migration_failed;

    return result;
}

}
