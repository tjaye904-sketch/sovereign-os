#pragma once

#include <cstdint>

namespace sovereign
{

enum class EventCategory : std::uint8_t
{
    Runtime = 0,
    Executive,
    Memory,
    Reasoning,
    Learning,
    Repository,
    Security,
    Platform,
    Device,
    Communication,
    Knowledge,
    Federation,
    User,
    Diagnostic
};

} // namespace sovereign
