#pragma once

#include <cstdint>

namespace sovereign
{

enum class EventPriority : std::uint8_t
{
    Background = 0,
    Low,
    Normal,
    High,
    Critical,
    Emergency
};

} // namespace sovereign
