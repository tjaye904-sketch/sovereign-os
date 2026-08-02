#pragma once

#include <cstdint>

namespace sovereign
{

enum class EventSecurity : std::uint8_t
{
    Public = 0,
    Internal,
    Restricted,
    Confidential,
    Executive,
    Constitutional
};

} // namespace sovereign
