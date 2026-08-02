#include "runtime_service_capability_trace_report_migration_dispatch_diagnostic_serialization.hpp"

#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include "test_require.hpp"

namespace
{

sovereign::RuntimeServiceCapabilityPipelineTrace
Trace(
    const std::uint64_t sequence,
    const std::uint8_t status)
{
    sovereign::RuntimeServiceCapabilityPipelineTrace trace;

    trace.invocation_sequence = sequence;
    trace.capability = "capability";
    trace.operation = "execute";
    trace.selected_service_name = "provider";
    trace.result_status = status;

    return trace;
}

std::string CurrentPayload()
{
    using namespace sovereign;

    std::deque<RuntimeServiceCapabilityPipelineTrace> traces;

    traces.push_back(Trace(1U, 0U));
    traces.push_back(Trace(2U, 10U));
    traces.push_back(Trace(3U, 20U));
    traces.push_back(Trace(4U, 30U));

    RuntimeServiceCapabilityTraceQuery query;
    query.capability = "capability";

    const auto summary =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            query);

    const auto serialized =
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            summary.summary);

    REQUIRE(serialized.serialization_available);
    REQUIRE(!serialized.payload.empty());

    return serialized.payload;
}

bool Contains(
    const std::string_view text,
    const std::string_view expected)
{
    return text.find(expected) != std::string_view::npos;
}

}

int main()
{
    using namespace sovereign;

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics
            diagnostics;

        diagnostics.stable_code.clear();
        diagnostics.message = "message";

        const auto serialized =
            SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics(
                diagnostics);

        REQUIRE(!serialized.serialization_available);
        REQUIRE(serialized.payload.empty());
        REQUIRE(!serialized.diagnostic.empty());
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics
            diagnostics;

        diagnostics.stable_code =
            "capability_trace.report_migration.test";

        diagnostics.message.clear();

        const auto serialized =
            SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics(
                diagnostics);

        REQUIRE(!serialized.serialization_available);
        REQUIRE(serialized.payload.empty());
        REQUIRE(!serialized.diagnostic.empty());
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics
            diagnostics;

        diagnostics.stable_code =
            "capability_trace.report_migration.synthetic";

        diagnostics.message =
            "quoted \"message\"\nnext line";

        diagnostics.source_version = 0U;
        diagnostics.target_version = 1U;

        diagnostics.registry_consulted = true;
        diagnostics.migration_path_available = true;
        diagnostics.resolved_path_steps = 2U;

        diagnostics.migration_execution_succeeded = false;
        diagnostics.completed_migration_steps = 1U;

        diagnostics.post_migration_validation_succeeded = false;
        diagnostics.dispatch_succeeded = false;
        diagnostics.payload_available = false;

        diagnostics.underlying_diagnostic =
            "handler\\failure\tcode";

        const auto serialized =
            SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnostics(
                diagnostics);

        REQUIRE(serialized.serialization_available);
        REQUIRE(serialized.diagnostic.empty());

        REQUIRE(
            serialized.payload ==
            "{\"diagnostic_schema_version\":1,"
            "\"stable_code\":\"capability_trace.report_migration.synthetic\","
            "\"message\":\"quoted \\\"message\\\"\\nnext line\","
            "\"source_version\":0,"
            "\"target_version\":1,"
            "\"registry_consulted\":true,"
            "\"migration_path_available\":true,"
            "\"resolved_path_steps\":2,"
            "\"migration_execution_succeeded\":false,"
            "\"completed_migration_steps\":1,"
            "\"post_migration_validation_succeeded\":false,"
            "\"dispatch_succeeded\":false,"
            "\"payload_available\":false,"
            "\"underlying_diagnostic\":\"handler\\\\failure\\tcode\"}");
    }

    const std::string current_payload =
        CurrentPayload();

    {
        const auto serialized =
            DispatchDiagnoseAndSerializeRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                current_payload);

        REQUIRE(serialized.serialization_available);
        REQUIRE(serialized.diagnostic.empty());

        REQUIRE(
            Contains(
                serialized.payload,
                "\"diagnostic_schema_version\":1"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"stable_code\":\"capability_trace.report_migration.current_schema_ready\""));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"source_version\":1"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"target_version\":1"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"registry_consulted\":false"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"dispatch_succeeded\":true"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"payload_available\":true"));
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
            registry;

        REQUIRE(
            registry.Register(
                0U,
                1U,
                [&current_payload](
                    const std::string_view)
                {
                    return RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult{
                        true,
                        current_payload,
                        {}};
                }).registered);

        const auto serialized =
            DispatchDiagnoseAndSerializeRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":0,"legacy":true})",
                registry);

        REQUIRE(serialized.serialization_available);

        REQUIRE(
            Contains(
                serialized.payload,
                "\"stable_code\":\"capability_trace.report_migration.legacy_schema_migrated\""));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"registry_consulted\":true"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"migration_path_available\":true"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"resolved_path_steps\":1"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"migration_execution_succeeded\":true"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"completed_migration_steps\":1"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"post_migration_validation_succeeded\":true"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"dispatch_succeeded\":true"));
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
            registry;

        REQUIRE(
            registry.Register(
                0U,
                1U,
                [](
                    const std::string_view)
                {
                    return RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult{
                        false,
                        {},
                        "failure \"with quotes\""};
                }).registered);

        const auto serialized =
            DispatchDiagnoseAndSerializeRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":0})",
                registry);

        REQUIRE(serialized.serialization_available);

        REQUIRE(
            Contains(
                serialized.payload,
                "\"stable_code\":\"capability_trace.report_migration.legacy_migration_failed\""));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"migration_execution_succeeded\":false"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"underlying_diagnostic\":\"failure \\\"with quotes\\\"\""));
    }

    {
        const auto serialized =
            DispatchDiagnoseAndSerializeRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":2})");

        REQUIRE(serialized.serialization_available);

        REQUIRE(
            Contains(
                serialized.payload,
                "\"stable_code\":\"capability_trace.report_migration.future_schema_unsupported\""));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"source_version\":2"));

        REQUIRE(
            Contains(
                serialized.payload,
                "\"dispatch_succeeded\":false"));
    }

    return 0;
}
