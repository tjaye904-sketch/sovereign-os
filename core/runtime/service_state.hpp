#pragma once

#include <cstdint>

namespace sovereign
{

enum class ServiceState : std::uint8_t
{
    Created = 0,
    Starting,
    Running,
    Stopping,
    Stopped,
    Failed
};

} // namespace sovereign
