#pragma once

#include <cstdint>

namespace sovereign
{

enum class HealthState : std::uint8_t
{
    Unknown = 0,
    Healthy,
    Degraded,
    Unhealthy,
    Critical
};

} // namespace sovereign
