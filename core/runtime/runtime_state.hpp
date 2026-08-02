#pragma once

#include <cstdint>

namespace sovereign
{

enum class RuntimeState : std::uint8_t
{
    Offline = 0,
    Initializing,
    Starting,
    Running,
    Pausing,
    Paused,
    Resuming,
    Stopping,
    Stopped,
    Faulted
};

} // namespace sovereign
