#pragma once

#include "runtime_service_capability_trace_report_migration_dispatcher.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace sovereign
{

enum class RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticCode
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

struct RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticCode
        code{
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticCode::
                migration_failed};

    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus
        dispatch_status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus::
                migration_failed};

    RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus
        schema_status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus::
                malformed};

    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathStatus
        path_status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathStatus::
                invalid_version};

    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionStatus
        execution_status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionStatus::
                path_resolution_failed};

    std::uint32_t source_version{0};
    std::uint32_t target_version{
        RuntimeServiceCapabilityTraceDistributionExplanationCurrentSchemaVersion};

    std::size_t resolved_path_steps{0};
    std::size_t completed_migration_steps{0};

    bool registry_consulted{false};
    bool migration_path_available{false};
    bool migration_execution_succeeded{false};
    bool post_migration_validation_succeeded{false};
    bool dispatch_succeeded{false};
    bool payload_available{false};

    std::string stable_code{};
    std::string message{};
    std::string underlying_diagnostic{};
};

inline std::string_view
RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticStableCode(
    const RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticCode
        code) noexcept
{
    using Code =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticCode;

    switch (code)
    {
        case Code::current_schema_ready:
            return "capability_trace.report_migration.current_schema_ready";

        case Code::current_schema_validation_failed:
            return "capability_trace.report_migration.current_schema_validation_failed";

        case Code::legacy_schema_unsupported:
            return "capability_trace.report_migration.legacy_schema_unsupported";

        case Code::legacy_schema_migrated:
            return "capability_trace.report_migration.legacy_schema_migrated";

        case Code::legacy_migration_path_unavailable:
            return "capability_trace.report_migration.legacy_migration_path_unavailable";

        case Code::legacy_migration_failed:
            return "capability_trace.report_migration.legacy_migration_failed";

        case Code::legacy_migrated_payload_validation_failed:
            return "capability_trace.report_migration.legacy_migrated_payload_validation_failed";

        case Code::future_schema_unsupported:
            return "capability_trace.report_migration.future_schema_unsupported";

        case Code::schema_version_missing:
            return "capability_trace.report_migration.schema_version_missing";

        case Code::schema_version_malformed:
            return "capability_trace.report_migration.schema_version_malformed";

        case Code::migration_failed:
            return "capability_trace.report_migration.failed";
    }

    return "capability_trace.report_migration.failed";
}

inline std::string_view
RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticMessage(
    const RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticCode
        code) noexcept
{
    using Code =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticCode;

    switch (code)
    {
        case Code::current_schema_ready:
            return "The report already uses the current schema and is valid.";

        case Code::current_schema_validation_failed:
            return "The report declares the current schema but failed validation.";

        case Code::legacy_schema_unsupported:
            return "The report uses a legacy schema and no migration registry was supplied.";

        case Code::legacy_schema_migrated:
            return "The legacy report was migrated and validated successfully.";

        case Code::legacy_migration_path_unavailable:
            return "No complete migration path exists from the source schema to the current schema.";

        case Code::legacy_migration_failed:
            return "A registered legacy migration step failed.";

        case Code::legacy_migrated_payload_validation_failed:
            return "The migration completed, but the resulting current-schema payload failed validation.";

        case Code::future_schema_unsupported:
            return "The report uses a schema newer than this runtime supports.";

        case Code::schema_version_missing:
            return "The report does not contain a schema version.";

        case Code::schema_version_malformed:
            return "The report schema version is malformed.";

        case Code::migration_failed:
            return "The report migration dispatch failed.";
    }

    return "The report migration dispatch failed.";
}

inline RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticCode
ClassifyRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostic(
    const RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus
        status) noexcept
{
    using DispatchStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus;

    using Code =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticCode;

    switch (status)
    {
        case DispatchStatus::current_schema_ready:
            return Code::current_schema_ready;

        case DispatchStatus::current_schema_validation_failed:
            return Code::current_schema_validation_failed;

        case DispatchStatus::legacy_schema_unsupported:
            return Code::legacy_schema_unsupported;

        case DispatchStatus::legacy_schema_migrated:
            return Code::legacy_schema_migrated;

        case DispatchStatus::legacy_migration_path_unavailable:
            return Code::legacy_migration_path_unavailable;

        case DispatchStatus::legacy_migration_failed:
            return Code::legacy_migration_failed;

        case DispatchStatus::legacy_migrated_payload_validation_failed:
            return Code::legacy_migrated_payload_validation_failed;

        case DispatchStatus::future_schema_unsupported:
            return Code::future_schema_unsupported;

        case DispatchStatus::schema_version_missing:
            return Code::schema_version_missing;

        case DispatchStatus::schema_version_malformed:
            return Code::schema_version_malformed;

        case DispatchStatus::migration_failed:
            return Code::migration_failed;
    }

    return Code::migration_failed;
}

inline RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics
DiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatch(
    const RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchResult&
        dispatch)
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics
        diagnostics;

    diagnostics.dispatch_status = dispatch.status;
    diagnostics.schema_status = dispatch.schema_probe.status;

    diagnostics.source_version = dispatch.source_version;
    diagnostics.target_version = dispatch.target_version;

    diagnostics.registry_consulted = dispatch.registry_consulted;
    diagnostics.dispatch_succeeded = dispatch.dispatch_succeeded;
    diagnostics.payload_available = dispatch.payload_available;

    diagnostics.path_status =
        dispatch.registry_execution.path.status;

    diagnostics.execution_status =
        dispatch.registry_execution.status;

    diagnostics.resolved_path_steps =
        dispatch.registry_execution.path.steps.size();

    diagnostics.completed_migration_steps =
        dispatch.registry_execution.completed_steps;

    diagnostics.migration_path_available =
        dispatch.registry_execution.path.path_available;

    diagnostics.migration_execution_succeeded =
        dispatch.registry_execution.migration_succeeded;

    diagnostics.post_migration_validation_succeeded =
        dispatch.dispatch_succeeded &&
        dispatch.payload_available;

    diagnostics.code =
        ClassifyRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostic(
            dispatch.status);

    diagnostics.stable_code =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticStableCode(
            diagnostics.code);

    diagnostics.message =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticMessage(
            diagnostics.code);

    diagnostics.underlying_diagnostic =
        dispatch.diagnostic;

    return diagnostics;
}

inline RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics
DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
    const std::string_view payload)
{
    return DiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatch(
        DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
            payload));
}

inline RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics
DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
    const std::string_view payload,
    const RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry&
        registry,
    const std::size_t maximum_migration_hops = 64U)
{
    return DiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatch(
        DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
            payload,
            registry,
            maximum_migration_hops));
}

}
