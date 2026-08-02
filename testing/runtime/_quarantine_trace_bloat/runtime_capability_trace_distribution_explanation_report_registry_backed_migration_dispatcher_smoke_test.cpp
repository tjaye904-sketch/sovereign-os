#include "runtime_service_capability_trace_report_migration_dispatcher.hpp"

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

    traces.push_back(Trace(1, 0));
    traces.push_back(Trace(2, 10));
    traces.push_back(Trace(3, 20));
    traces.push_back(Trace(4, 30));

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

    using RegistrationStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistrationStatus;

    using DispatchStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus;

    const std::string current_payload =
        CurrentPayload();

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
            registry;

        const auto registration =
            registry.Register(
                0U,
                1U,
                [&current_payload](
                    const std::string_view payload)
                {
                    REQUIRE(
                        payload ==
                        R"({"schema_version":0,"legacy":true})");

                    return RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult{
                        true,
                        current_payload,
                        {}};
                });

        REQUIRE(
            registration.status ==
            RegistrationStatus::registered);

        REQUIRE(registration.registered);
        REQUIRE(registry.Contains(0U));

        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":0,"legacy":true})",
                registry);

        REQUIRE(
            dispatched.status ==
            DispatchStatus::legacy_schema_migrated);

        REQUIRE(dispatched.registry_consulted);
        REQUIRE(dispatched.registry_execution.migration_succeeded);
        REQUIRE(dispatched.registry_execution.completed_steps == 1U);

        REQUIRE(dispatched.source_version == 0U);
        REQUIRE(dispatched.target_version == 1U);

        REQUIRE(dispatched.dispatch_succeeded);
        REQUIRE(dispatched.payload_available);
        REQUIRE(!dispatched.canonical_payload.empty());

        const auto deserialized =
            DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
                dispatched.canonical_payload);

        REQUIRE(deserialized.deserialization_available);

        const auto canonical =
            SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportView(
                deserialized.report);

        REQUIRE(canonical.serialization_available);

        REQUIRE(
            canonical.payload ==
            dispatched.canonical_payload);
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
            registry;

        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":0})",
                registry);

        REQUIRE(
            dispatched.status ==
            DispatchStatus::legacy_migration_path_unavailable);

        REQUIRE(dispatched.registry_consulted);
        REQUIRE(!dispatched.dispatch_succeeded);
        REQUIRE(!dispatched.payload_available);
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
                        "synthetic legacy migration failure"};
                }).registered);

        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":0})",
                registry);

        REQUIRE(
            dispatched.status ==
            DispatchStatus::legacy_migration_failed);

        REQUIRE(dispatched.registry_consulted);
        REQUIRE(!dispatched.dispatch_succeeded);
        REQUIRE(!dispatched.payload_available);

        REQUIRE(
            dispatched.diagnostic ==
            "synthetic legacy migration failure");
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

        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":0})",
                registry);

        REQUIRE(
            dispatched.status ==
            DispatchStatus::
                legacy_migrated_payload_validation_failed);

        REQUIRE(dispatched.registry_consulted);
        REQUIRE(dispatched.registry_execution.migration_succeeded);
        REQUIRE(!dispatched.dispatch_succeeded);
        REQUIRE(!dispatched.payload_available);
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
            registry;

        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                current_payload,
                registry);

        REQUIRE(
            dispatched.status ==
            DispatchStatus::current_schema_ready);

        REQUIRE(!dispatched.registry_consulted);
        REQUIRE(dispatched.dispatch_succeeded);
        REQUIRE(dispatched.payload_available);
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
            registry;

        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":2})",
                registry);

        REQUIRE(
            dispatched.status ==
            DispatchStatus::future_schema_unsupported);

        REQUIRE(!dispatched.registry_consulted);
        REQUIRE(!dispatched.dispatch_succeeded);
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
            registry;

        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                "{}",
                registry);

        REQUIRE(
            dispatched.status ==
            DispatchStatus::schema_version_missing);

        REQUIRE(!dispatched.registry_consulted);
        REQUIRE(!dispatched.dispatch_succeeded);
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
            registry;

        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":999999999999999999999999999999999999})",
                registry);

        REQUIRE(
            dispatched.status ==
            DispatchStatus::schema_version_malformed);

        REQUIRE(!dispatched.registry_consulted);
        REQUIRE(!dispatched.dispatch_succeeded);
    }

    return 0;
}
