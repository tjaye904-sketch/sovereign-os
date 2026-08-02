#include "runtime_service_capability_trace_report_migration_registry.hpp"

#include <string>
#include <string_view>
#include "test_require.hpp"

int main()
{
    using namespace sovereign;

    using RegistrationStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistrationStatus;

    using PathStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathStatus;

    using ExecutionStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionStatus;

    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
        registry;

    {
        const auto result =
            registry.Register(
                1U,
                2U,
                [](
                    const std::string_view payload)
                {
                    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult
                        step;

                    step.succeeded = true;
                    step.payload =
                        std::string(payload) + "|v2";

                    return step;
                });

        REQUIRE(result.status == RegistrationStatus::registered);
        REQUIRE(result.registered);
        REQUIRE(registry.Contains(1U));
        REQUIRE(registry.Size() == 1U);
    }

    {
        const auto result =
            registry.Register(
                2U,
                3U,
                [](
                    const std::string_view payload)
                {
                    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult
                        step;

                    step.succeeded = true;
                    step.payload =
                        std::string(payload) + "|v3";

                    return step;
                });

        REQUIRE(result.status == RegistrationStatus::registered);
        REQUIRE(result.registered);
        REQUIRE(registry.Contains(2U));
        REQUIRE(registry.Size() == 2U);
    }

    {
        const auto result =
            registry.Register(
                1U,
                4U,
                [](
                    const std::string_view payload)
                {
                    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult
                        step;

                    step.succeeded = true;
                    step.payload = std::string(payload);

                    return step;
                });

        REQUIRE(
            result.status ==
            RegistrationStatus::duplicate_source_version);

        REQUIRE(!result.registered);
        REQUIRE(registry.Size() == 2U);
    }

    {
        const auto path =
            registry.Resolve(
                1U,
                3U);

        REQUIRE(path.status == PathStatus::resolved);
        REQUIRE(path.path_available);
        REQUIRE(path.steps.size() == 2U);
        REQUIRE(path.visited_versions.size() == 3U);
        REQUIRE(path.visited_versions[0] == 1U);
        REQUIRE(path.visited_versions[1] == 2U);
        REQUIRE(path.visited_versions[2] == 3U);
    }

    {
        const auto execution =
            registry.Execute(
                1U,
                3U,
                "report");

        REQUIRE(execution.status == ExecutionStatus::migrated);
        REQUIRE(execution.migration_succeeded);
        REQUIRE(execution.payload_available);
        REQUIRE(execution.completed_steps == 2U);
        REQUIRE(execution.migrated_payload == "report|v2|v3");
    }

    {
        const auto execution =
            registry.Execute(
                3U,
                3U,
                "current");

        REQUIRE(
            execution.status ==
            ExecutionStatus::already_at_target);

        REQUIRE(execution.migration_succeeded);
        REQUIRE(execution.payload_available);
        REQUIRE(execution.completed_steps == 0U);
        REQUIRE(execution.migrated_payload == "current");
    }

    {
        const auto path =
            registry.Resolve(
                3U,
                4U);

        REQUIRE(
            path.status ==
            PathStatus::missing_transition);

        REQUIRE(!path.path_available);
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
            cyclic_registry;

        REQUIRE(
            cyclic_registry.Register(
                1U,
                2U,
                [](
                    const std::string_view payload)
                {
                    return RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult{
                        true,
                        std::string(payload),
                        {}};
                }).registered);

        REQUIRE(
            cyclic_registry.Register(
                2U,
                1U,
                [](
                    const std::string_view payload)
                {
                    return RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult{
                        true,
                        std::string(payload),
                        {}};
                }).registered);

        const auto path =
            cyclic_registry.Resolve(
                1U,
                3U);

        REQUIRE(path.status == PathStatus::cycle_detected);
        REQUIRE(!path.path_available);
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
            failing_registry;

        REQUIRE(
            failing_registry.Register(
                1U,
                2U,
                [](
                    const std::string_view)
                {
                    return RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult{
                        false,
                        {},
                        "synthetic migration failure"};
                }).registered);

        const auto execution =
            failing_registry.Execute(
                1U,
                2U,
                "report");

        REQUIRE(
            execution.status ==
            ExecutionStatus::handler_failed);

        REQUIRE(!execution.migration_succeeded);
        REQUIRE(!execution.payload_available);
        REQUIRE(execution.completed_steps == 0U);
        REQUIRE(
            execution.diagnostic ==
            "synthetic migration failure");
    }

    {
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
            bounded_registry;

        REQUIRE(
            bounded_registry.Register(
                1U,
                2U,
                [](
                    const std::string_view payload)
                {
                    return RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult{
                        true,
                        std::string(payload),
                        {}};
                }).registered);

        REQUIRE(
            bounded_registry.Register(
                2U,
                3U,
                [](
                    const std::string_view payload)
                {
                    return RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult{
                        true,
                        std::string(payload),
                        {}};
                }).registered);

        const auto path =
            bounded_registry.Resolve(
                1U,
                3U,
                1U);

        REQUIRE(
            path.status ==
            PathStatus::hop_limit_exceeded);

        REQUIRE(!path.path_available);
    }

    return 0;
}
