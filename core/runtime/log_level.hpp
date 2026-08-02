#pragma once

#include <cstdint>

namespace sovereign
{

enum class LogLevel : std::uint8_t
{
    Trace = 0,
    Debug,
    Information,
    Warning,
    Error,
    Critical
};

} // namespace sovereign
