#pragma once

#include "log_record.hpp"

namespace sovereign
{

class LogSink
{
public:
    virtual ~LogSink() = default;

    LogSink(const LogSink&) = delete;
    LogSink& operator=(const LogSink&) = delete;

    LogSink(LogSink&&) = delete;
    LogSink& operator=(LogSink&&) = delete;

    virtual bool Write(
        const LogRecord& record) = 0;

protected:
    LogSink() = default;
};

} // namespace sovereign
