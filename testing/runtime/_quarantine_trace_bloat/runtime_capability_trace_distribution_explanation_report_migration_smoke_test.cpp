#include "runtime_service_capability_trace_report_migration.hpp"

#include <cstdint>
#include <deque>
#include "test_require.hpp"

namespace
{

sovereign::RuntimeServiceCapabilityPipelineTrace
Trace(
    std::uint64_t seq,
    std::uint8_t status)
{
    sovereign::RuntimeServiceCapabilityPipelineTrace t;

    t.invocation_sequence = seq;
    t.capability = "capability";
    t.operation = "execute";
    t.selected_service_name = "provider";
    t.result_status = status;

    return t;
}

}

int main()
{
    using namespace sovereign;

    std::deque<RuntimeServiceCapabilityPipelineTrace> traces;

    traces.push_back(Trace(1,0));
    traces.push_back(Trace(2,0));
    traces.push_back(Trace(3,0));
    traces.push_back(Trace(4,10));
    traces.push_back(Trace(5,20));
    traces.push_back(Trace(6,30));

    RuntimeServiceCapabilityTraceQuery query;
    query.capability="capability";

    auto summary =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            query);

    auto serialized =
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            summary.summary);

    REQUIRE(serialized.serialization_available);

    auto migrated =
        MigrateRuntimeServiceCapabilityTraceDistributionExplanationReport(
            serialized.payload);

    REQUIRE(migrated.migrated);

    REQUIRE(
        migrated.status==
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStatus::
            already_current);

    auto deserialized =
        DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            migrated.migrated_payload);

    REQUIRE(deserialized.deserialization_available);

    auto canonical =
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportView(
            deserialized.report);

    REQUIRE(canonical.serialization_available);

    REQUIRE(
        canonical.payload==
        migrated.migrated_payload);

    auto malformed =
        MigrateRuntimeServiceCapabilityTraceDistributionExplanationReport(
            "{");

    REQUIRE(
        malformed.status==
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStatus::
            malformed_payload);

    return 0;
}
