#pragma once

#include "health_state.hpp"

#include <chrono>
#include <string>

namespace sovereign
{

struct HealthRecord final
{
    std::chrono::system_clock::time_point timestamp{
        std::chrono::system_clock::now()};

    HealthState state{HealthState::Unknown};

    std::string component;

    std::string message;
};

} // namespace sovereign
