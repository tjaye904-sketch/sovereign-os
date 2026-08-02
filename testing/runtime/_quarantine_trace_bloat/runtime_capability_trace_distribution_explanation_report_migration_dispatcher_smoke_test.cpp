#include "runtime_service_capability_trace_report_migration_dispatcher.hpp"

#include <cstdint>
#include <deque>
#include <string>
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

}

int main()
{
    using namespace sovereign;

    using DispatchStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationDispatchStatus;

    {
        std::deque<RuntimeServiceCapabilityPipelineTrace> traces;

        traces.push_back(Trace(1, 0));
        traces.push_back(Trace(2, 0));
        traces.push_back(Trace(3, 10));
        traces.push_back(Trace(4, 20));
        traces.push_back(Trace(5, 30));

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

        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                serialized.payload);

        REQUIRE(
            dispatched.status ==
            DispatchStatus::current_schema_ready);

        REQUIRE(dispatched.dispatch_succeeded);
        REQUIRE(dispatched.payload_available);
        REQUIRE(dispatched.source_version == 1U);
        REQUIRE(dispatched.target_version == 1U);
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
        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":2})");

        REQUIRE(
            dispatched.status ==
            DispatchStatus::future_schema_unsupported);

        REQUIRE(dispatched.source_version == 2U);
        REQUIRE(!dispatched.dispatch_succeeded);
        REQUIRE(!dispatched.payload_available);
    }

    {
        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":0})");

        REQUIRE(
            dispatched.status ==
            DispatchStatus::legacy_schema_unsupported);

        REQUIRE(dispatched.source_version == 0U);
        REQUIRE(!dispatched.dispatch_succeeded);
        REQUIRE(!dispatched.payload_available);
    }

    {
        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                "{}");

        REQUIRE(
            dispatched.status ==
            DispatchStatus::schema_version_missing);

        REQUIRE(!dispatched.schema_probe.schema_found);
        REQUIRE(!dispatched.dispatch_succeeded);
    }

    {
        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":999999999999999999999999999999999999})");

        REQUIRE(
            dispatched.status ==
            DispatchStatus::schema_version_malformed);

        REQUIRE(dispatched.schema_probe.schema_found);
        REQUIRE(!dispatched.dispatch_succeeded);
    }

    {
        const auto dispatched =
            DispatchRuntimeServiceCapabilityTraceDistributionExplanationReportMigration(
                R"({"schema_version":1})");

        REQUIRE(
            dispatched.status ==
            DispatchStatus::current_schema_validation_failed);

        REQUIRE(dispatched.source_version == 1U);
        REQUIRE(!dispatched.dispatch_succeeded);
        REQUIRE(!dispatched.payload_available);
    }

    return 0;
}
