#include "runtime_service_capability_trace_report_schema_version_detection.hpp"

#include "test_require.hpp"

using namespace sovereign;

int main()
{
    {
        auto r =
            ProbeRuntimeServiceCapabilityTraceDistributionExplanationReportSchemaVersion(
                R"({"schema_version":1})");

        REQUIRE(r.schema_found);
        REQUIRE(r.detected_version == 1);
        REQUIRE(
            r.status ==
            RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus::current);
    }

    {
        auto r =
            ProbeRuntimeServiceCapabilityTraceDistributionExplanationReportSchemaVersion(
                R"({"schema_version":2})");

        REQUIRE(r.schema_found);
        REQUIRE(r.detected_version == 2);
        REQUIRE(
            r.status ==
            RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus::future);
    }

    {
        auto r =
            ProbeRuntimeServiceCapabilityTraceDistributionExplanationReportSchemaVersion(
                R"({"schema_version":0})");

        REQUIRE(r.schema_found);
        REQUIRE(r.detected_version == 0);
        REQUIRE(
            r.status ==
            RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus::unsupported);
    }

    {
        auto r =
            ProbeRuntimeServiceCapabilityTraceDistributionExplanationReportSchemaVersion(
                "{}");

        REQUIRE(!r.schema_found);
        REQUIRE(
            r.status ==
            RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus::missing);
    }

    return 0;
}
