#include "runtime_service_capability_trace_report_descriptor.hpp"

#include <cassert>
#include <cstdint>
#include "test_require.hpp"

using namespace sovereign;

int main()
{
    using Descriptor =
        RuntimeServiceCapabilityTraceReportDescriptor;

    using Feature =
        RuntimeServiceCapabilityTraceReportFeature;

    static_assert(!Descriptor::Identifier().empty());
    static_assert(!Descriptor::DisplayName().empty());
    static_assert(!Descriptor::RuntimeVersion().empty());

    static_assert(Descriptor::SchemaVersion() == 1U);

    static_assert(
        Descriptor::MinimumCompatibleSchemaVersion() <=
        Descriptor::SchemaVersion());

    static_assert(
        Descriptor::MaximumCompatibleSchemaVersion() >=
        Descriptor::SchemaVersion());

    static_assert(
        Descriptor::MigrationGeneration() == 20U);

    static_assert(
        Descriptor::Supports(Feature::Query));

    static_assert(
        Descriptor::Supports(
            Feature::ResultStatusDistribution));

    static_assert(
        Descriptor::Supports(
            Feature::DistributionStatistics));

    static_assert(
        Descriptor::Supports(
            Feature::DistributionDiagnostics));

    static_assert(
        Descriptor::Supports(
            Feature::ShapeClassification));

    static_assert(
        Descriptor::Supports(
            Feature::ShapeExplanation));

    static_assert(
        Descriptor::Supports(Feature::Persistence));

    static_assert(
        Descriptor::Supports(Feature::Migration));

    static_assert(
        Descriptor::Supports(
            Feature::MigrationDiagnostics));

    static_assert(
        Descriptor::Supports(
            Feature::RecoveryVerification));

    static_assert(
        !Descriptor::Supports(Feature::None));

    static_assert(
        Descriptor::IsSchemaCompatible(1U));

    static_assert(
        !Descriptor::IsSchemaCompatible(0U));

    static_assert(
        !Descriptor::IsSchemaCompatible(2U));

    constexpr auto combined =
        Feature::Query |
        Feature::Persistence |
        Feature::Migration;

    REQUIRE(Descriptor::Supports(combined));

    const auto feature_mask =
        static_cast<std::uint32_t>(
            Descriptor::Features());

    REQUIRE(feature_mask != 0U);

    return 0;
}
