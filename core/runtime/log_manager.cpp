#include "log_manager.hpp"

#include <utility>

namespace sovereign
{

bool LogManager::RegisterSink(
    std::shared_ptr<LogSink> sink)
{
    if (!sink)
    {
        return false;
    }

    std::scoped_lock lock(mutex_);

    sinks_.push_back(std::move(sink));

    return true;
}

bool LogManager::Write(
    LogRecord record)
{
    std::vector<std::shared_ptr<LogSink>> sinks;

    {
        std::scoped_lock lock(mutex_);
        sinks = sinks_;
    }

    bool success = true;

    for (const auto& sink : sinks)
    {
        if (!sink->Write(record))
        {
            success = false;
        }
    }

    return success;
}

std::size_t LogManager::SinkCount() const
{
    std::scoped_lock lock(mutex_);

    return sinks_.size();
}

} // namespace sovereign
