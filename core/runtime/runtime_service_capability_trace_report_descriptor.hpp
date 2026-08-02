#pragma once

#include <cstdint>
#include <string_view>

namespace sovereign
{

enum class RuntimeServiceCapabilityTraceReportFeature :
    std::uint32_t
{
    None = 0U,

    Query = 1U << 0U,
    ResultStatusDistribution = 1U << 1U,
    DistributionStatistics = 1U << 2U,
    DistributionDiagnostics = 1U << 3U,
    ShapeClassification = 1U << 4U,
    ShapeExplanation = 1U << 5U,
    Persistence = 1U << 6U,
    Migration = 1U << 7U,
    MigrationDiagnostics = 1U << 8U,
    RecoveryVerification = 1U << 9U
};

[[nodiscard]]
constexpr RuntimeServiceCapabilityTraceReportFeature
operator|(
    RuntimeServiceCapabilityTraceReportFeature left,
    RuntimeServiceCapabilityTraceReportFeature right) noexcept
{
    return static_cast<
        RuntimeServiceCapabilityTraceReportFeature>(
            static_cast<std::uint32_t>(left) |
            static_cast<std::uint32_t>(right));
}

[[nodiscard]]
constexpr RuntimeServiceCapabilityTraceReportFeature
operator&(
    RuntimeServiceCapabilityTraceReportFeature left,
    RuntimeServiceCapabilityTraceReportFeature right) noexcept
{
    return static_cast<
        RuntimeServiceCapabilityTraceReportFeature>(
            static_cast<std::uint32_t>(left) &
            static_cast<std::uint32_t>(right));
}

struct RuntimeServiceCapabilityTraceReportDescriptor final
{
    static constexpr std::string_view Identifier()
        noexcept
    {
        return
            "sovereign.runtime."
            "service_capability_trace_report";
    }

    static constexpr std::string_view DisplayName()
        noexcept
    {
        return "Runtime Service Capability Trace Report";
    }

    static constexpr std::string_view RuntimeVersion()
        noexcept
    {
        return "0.1.0";
    }

    static constexpr std::uint32_t SchemaVersion()
        noexcept
    {
        return 1U;
    }

    static constexpr std::uint32_t
    MinimumCompatibleSchemaVersion() noexcept
    {
        return 1U;
    }

    static constexpr std::uint32_t
    MaximumCompatibleSchemaVersion() noexcept
    {
        return SchemaVersion();
    }

    static constexpr std::uint32_t
    MigrationGeneration() noexcept
    {
        /*
         * RT-150 through RT-169 established the current
         * capability-trace report migration lineage.
         */
        return 20U;
    }

    static constexpr
    RuntimeServiceCapabilityTraceReportFeature
    Features() noexcept
    {
        using Feature =
            RuntimeServiceCapabilityTraceReportFeature;

        return
            Feature::Query |
            Feature::ResultStatusDistribution |
            Feature::DistributionStatistics |
            Feature::DistributionDiagnostics |
            Feature::ShapeClassification |
            Feature::ShapeExplanation |
            Feature::Persistence |
            Feature::Migration |
            Feature::MigrationDiagnostics |
            Feature::RecoveryVerification;
    }

    [[nodiscard]]
    static constexpr bool Supports(
        RuntimeServiceCapabilityTraceReportFeature
            feature) noexcept
    {
        if (feature ==
            RuntimeServiceCapabilityTraceReportFeature::
                None)
        {
            return false;
        }

        return
            (static_cast<std::uint32_t>(
                 Features() & feature) ==
             static_cast<std::uint32_t>(feature));
    }

    [[nodiscard]]
    static constexpr bool IsSchemaCompatible(
        std::uint32_t schema_version) noexcept
    {
        return
            schema_version >=
                MinimumCompatibleSchemaVersion() &&
            schema_version <=
                MaximumCompatibleSchemaVersion();
    }
};

} // namespace sovereign
