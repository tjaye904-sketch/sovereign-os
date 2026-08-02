#pragma once

#include <cstdint>
#include <regex>
#include <string>
#include <string_view>

namespace sovereign
{

enum class RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus
{
    current,
    future,
    unsupported,
    missing,
    malformed
};

struct RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaProbe
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus
        status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus::malformed};

    std::uint32_t detected_version{0};
    bool schema_found{false};
};

inline constexpr std::uint32_t
RuntimeServiceCapabilityTraceDistributionExplanationCurrentSchemaVersion = 1U;

inline RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaProbe
ProbeRuntimeServiceCapabilityTraceDistributionExplanationReportSchemaVersion(
    std::string_view payload)
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaProbe result;

    static const std::regex pattern(
        R"("schema_version"\s*:\s*([0-9]+))");

    std::cmatch match;

    if (!std::regex_search(
            payload.begin(),
            payload.end(),
            match,
            pattern))
    {
        result.status =
            RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus::missing;
        return result;
    }

    result.schema_found = true;

    try
    {
        result.detected_version =
            static_cast<std::uint32_t>(
                std::stoul(match[1].str()));
    }
    catch (...)
    {
        result.status =
            RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus::malformed;
        return result;
    }

    if (result.detected_version ==
        RuntimeServiceCapabilityTraceDistributionExplanationCurrentSchemaVersion)
    {
        result.status =
            RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus::current;
    }
    else if (result.detected_version >
             RuntimeServiceCapabilityTraceDistributionExplanationCurrentSchemaVersion)
    {
        result.status =
            RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus::future;
    }
    else
    {
        result.status =
            RuntimeServiceCapabilityTraceDistributionExplanationReportSchemaStatus::unsupported;
    }

    return result;
}

}
