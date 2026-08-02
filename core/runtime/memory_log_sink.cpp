#include "memory_log_sink.hpp"

namespace sovereign
{

bool MemoryLogSink::Write(
    const LogRecord& record)
{
    std::scoped_lock lock(mutex_);

    records_.push_back(record);

    return true;
}

std::size_t MemoryLogSink::Size() const
{
    std::scoped_lock lock(mutex_);

    return records_.size();
}

std::vector<LogRecord>
MemoryLogSink::Records() const
{
    std::scoped_lock lock(mutex_);

    return records_;
}

void MemoryLogSink::Clear()
{
    std::scoped_lock lock(mutex_);

    records_.clear();
}

} // namespace sovereign
