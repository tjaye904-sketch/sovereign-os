#include "runtime_service_capability_trace_report_migration_dispatch_diagnostics.hpp"

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

}

int main()
{
    using namespace sovereign;

    using Code =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchDiagnosticCode;

    using PathStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathStatus;

    using ExecutionStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionStatus;

    const std::string current_payload =
        CurrentPayload();

    {
        const auto diagnostics =
            DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                current_payload);

        REQUIRE(diagnostics.code == Code::current_schema_ready);
        REQUIRE(
            diagnostics.stable_code ==
            "capability_trace.report_migration.current_schema_ready");

        REQUIRE(diagnostics.source_version == 1U);
        REQUIRE(diagnostics.target_version == 1U);

        REQUIRE(!diagnostics.registry_consulted);
        REQUIRE(diagnostics.dispatch_succeeded);
        REQUIRE(diagnostics.payload_available);
        REQUIRE(diagnostics.post_migration_validation_succeeded);
        REQUIRE(!diagnostics.message.empty());
    }

    {
        const auto diagnostics =
            DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":0})");

        REQUIRE(diagnostics.code == Code::legacy_schema_unsupported);
        REQUIRE(
            diagnostics.stable_code ==
            "capability_trace.report_migration.legacy_schema_unsupported");

        REQUIRE(diagnostics.source_version == 0U);
        REQUIRE(!diagnostics.registry_consulted);
        REQUIRE(!diagnostics.dispatch_succeeded);
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

        const auto diagnostics =
            DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":0,"legacy":true})",
                registry);

        REQUIRE(diagnostics.code == Code::legacy_schema_migrated);
        REQUIRE(
            diagnostics.stable_code ==
            "capability_trace.report_migration.legacy_schema_migrated");

        REQUIRE(diagnostics.registry_consulted);
        REQUIRE(diagnostics.migration_path_available);
        REQUIRE(diagnostics.migration_execution_succeeded);
        REQUIRE(diagnostics.post_migration_validation_succeeded);

        REQUIRE(diagnostics.path_status == PathStatus::resolved);
        REQUIRE(diagnostics.execution_status == ExecutionStatus::migrated);

        REQUIRE(diagnostics.resolved_path_steps == 1U);
        REQUIRE(diagnostics.completed_migration_steps == 1U);

        REQUIRE(diagnostics.dispatch_succeeded);
        REQUIRE(diagnostics.payload_available);
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
            registry;

        const auto diagnostics =
            DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":0})",
                registry);

        REQUIRE(
            diagnostics.code ==
            Code::legacy_migration_path_unavailable);

        REQUIRE(diagnostics.registry_consulted);
        REQUIRE(!diagnostics.migration_path_available);
        REQUIRE(!diagnostics.migration_execution_succeeded);

        REQUIRE(
            diagnostics.path_status ==
            PathStatus::missing_transition);

        REQUIRE(
            diagnostics.execution_status ==
            ExecutionStatus::path_resolution_failed);

        REQUIRE(diagnostics.completed_migration_steps == 0U);
        REQUIRE(!diagnostics.dispatch_succeeded);
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
                        "synthetic migration failure"};
                }).registered);

        const auto diagnostics =
            DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":0})",
                registry);

        REQUIRE(
            diagnostics.code ==
            Code::legacy_migration_failed);

        REQUIRE(diagnostics.registry_consulted);
        REQUIRE(diagnostics.migration_path_available);
        REQUIRE(!diagnostics.migration_execution_succeeded);

        REQUIRE(
            diagnostics.execution_status ==
            ExecutionStatus::handler_failed);

        REQUIRE(
            diagnostics.underlying_diagnostic ==
            "synthetic migration failure");

        REQUIRE(diagnostics.completed_migration_steps == 0U);
        REQUIRE(!diagnostics.dispatch_succeeded);
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
                        true,
                        R"({"schema_version":1})",
                        {}};
                }).registered);

        const auto diagnostics =
            DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":0})",
                registry);

        REQUIRE(
            diagnostics.code ==
            Code::legacy_migrated_payload_validation_failed);

        REQUIRE(diagnostics.registry_consulted);
        REQUIRE(diagnostics.migration_path_available);
        REQUIRE(diagnostics.migration_execution_succeeded);
        REQUIRE(!diagnostics.post_migration_validation_succeeded);

        REQUIRE(diagnostics.completed_migration_steps == 1U);
        REQUIRE(!diagnostics.dispatch_succeeded);
        REQUIRE(!diagnostics.payload_available);
    }

    {
        const auto diagnostics =
            DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":2})");

        REQUIRE(diagnostics.code == Code::future_schema_unsupported);
        REQUIRE(!diagnostics.registry_consulted);
        REQUIRE(!diagnostics.dispatch_succeeded);
    }

    {
        const auto diagnostics =
            DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                "{}");

        REQUIRE(diagnostics.code == Code::schema_version_missing);
        REQUIRE(!diagnostics.dispatch_succeeded);
    }

    {
        const auto diagnostics =
            DispatchAndDiagnoseRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":999999999999999999999999999999999999})");

        REQUIRE(diagnostics.code == Code::schema_version_malformed);
        REQUIRE(!diagnostics.dispatch_succeeded);
    }

    return 0;
}
