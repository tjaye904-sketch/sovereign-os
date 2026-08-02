#pragma once

#include "event_metadata.hpp"

#include <string>

namespace sovereign
{

struct Event final
{
    EventMetadata metadata;

    std::string topic;

    std::string payload;
};

} // namespace sovereign
