#pragma once

#include "runtime_service_capability_trace_report_migration_dispatch_diagnostics.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace sovereign
{

inline constexpr std::uint32_t
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticSerializationSchemaVersion{
        1U};

struct RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticSerializationResult
{
    bool serialization_available{false};
    std::string payload{};
    std::string diagnostic{};
};

inline std::string
EscapeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticJsonString(
    const std::string_view value)
{
    std::string escaped;
    escaped.reserve(value.size());

    constexpr char hexadecimal_digits[] = "0123456789abcdef";

    for (const unsigned char character : value)
    {
        switch (character)
        {
            case '"':
                escaped += "\\\"";
                break;

            case '\\':
                escaped += "\\\\";
                break;

            case '\b':
                escaped += "\\b";
                break;

            case '\f':
                escaped += "\\f";
                break;

            case '\n':
                escaped += "\\n";
                break;

            case '\r':
                escaped += "\\r";
                break;

            case '\t':
                escaped += "\\t";
                break;

            default:
            {
                if (character < 0x20U)
                {
                    escaped += "\\u00";
                    escaped += hexadecimal_digits[(character >> 4U) & 0x0FU];
                    escaped += hexadecimal_digits[character & 0x0FU];
                }
                else
                {
                    escaped.push_back(
                        static_cast<char>(character));
                }

                break;
            }
        }
    }

    return escaped;
}

inline std::string_view
RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticJsonBoolean(
    const bool value) noexcept
{
    return value ? "true" : "false";
}

inline RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticSerializationResult
SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics(
    const RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics&
        diagnostics)
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticSerializationResult
        result;

    if (diagnostics.stable_code.empty())
    {
        result.diagnostic =
            "Migration dispatch diagnostics cannot be serialized without a stable diagnostic code.";

        return result;
    }

    if (diagnostics.message.empty())
    {
        result.diagnostic =
            "Migration dispatch diagnostics cannot be serialized without a diagnostic message.";

        return result;
    }

    const std::string escaped_stable_code =
        EscapeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticJsonString(
            diagnostics.stable_code);

    const std::string escaped_message =
        EscapeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticJsonString(
            diagnostics.message);

    const std::string escaped_underlying_diagnostic =
        EscapeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticJsonString(
            diagnostics.underlying_diagnostic);

    std::string payload;
    payload.reserve(
        512U +
        escaped_stable_code.size() +
        escaped_message.size() +
        escaped_underlying_diagnostic.size());

    payload += "{\"diagnostic_schema_version\":";
    payload += std::to_string(
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticSerializationSchemaVersion);

    payload += ",\"stable_code\":\"";
    payload += escaped_stable_code;

    payload += "\",\"message\":\"";
    payload += escaped_message;

    payload += "\",\"source_version\":";
    payload += std::to_string(
        diagnostics.source_version);

    payload += ",\"target_version\":";
    payload += std::to_string(
        diagnostics.target_version);

    payload += ",\"registry_consulted\":";
    payload +=
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticJsonBoolean(
            diagnostics.registry_consulted);

    payload += ",\"migration_path_available\":";
    payload +=
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticJsonBoolean(
            diagnostics.migration_path_available);

    payload += ",\"resolved_path_steps\":";
    payload += std::to_string(
        diagnostics.resolved_path_steps);

    payload += ",\"migration_execution_succeeded\":";
    payload +=
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticJsonBoolean(
            diagnostics.migration_execution_succeeded);

    payload += ",\"completed_migration_steps\":";
    payload += std::to_string(
        diagnostics.completed_migration_steps);

    payload += ",\"post_migration_validation_succeeded\":";
    payload +=
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticJsonBoolean(
            diagnostics.post_migration_validation_succeeded);

    payload += ",\"dispatch_succeeded\":";
    payload +=
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticJsonBoolean(
            diagnostics.dispatch_succeeded);

    payload += ",\"payload_available\":";
    payload +=
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticJsonBoolean(
            diagnostics.payload_available);

    payload += ",\"underlying_diagnostic\":\"";
    payload += escaped_underlying_diagnostic;
    payload += "\"}";

    result.serialization_available = true;
    result.payload = std::move(payload);

    return result;
}

inline RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticSerializationResult
DispatchDiagnoseAndSerializeRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
    const std::string_view payload)
{
    return SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics(
        DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
            payload));
}

inline RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticSerializationResult
DispatchDiagnoseAndSerializeRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
    const std::string_view payload,
    const RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry&
        registry,
    const std::size_t maximum_migration_hops = 64U)
{
    return SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics(
        DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
            payload,
            registry,
            maximum_migration_hops));
}

}
