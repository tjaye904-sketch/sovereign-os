#pragma once

#include "log_level.hpp"

#include <chrono>
#include <string>

namespace sovereign
{

struct LogRecord final
{
    std::chrono::system_clock::time_point timestamp{
        std::chrono::system_clock::now()};

    LogLevel level{LogLevel::Information};

    std::string category;

    std::string message;
};

} // namespace sovereign
