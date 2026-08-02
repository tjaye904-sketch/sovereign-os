#pragma once

#include "log_sink.hpp"

#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

namespace sovereign
{

class LogManager final
{
public:
    bool RegisterSink(
        std::shared_ptr<LogSink> sink);

    bool Write(
        LogRecord record);

    [[nodiscard]]
    std::size_t SinkCount() const;

private:
    mutable std::mutex mutex_;

    std::vector<std::shared_ptr<LogSink>> sinks_;
};

} // namespace sovereign
