#pragma once

#include "log_sink.hpp"

#include <cstddef>
#include <mutex>
#include <vector>

namespace sovereign
{

class MemoryLogSink final : public LogSink
{
public:
    bool Write(
        const LogRecord& record) override;

    [[nodiscard]]
    std::size_t Size() const;

    [[nodiscard]]
    std::vector<LogRecord> Records() const;

    void Clear();

private:
    mutable std::mutex mutex_;

    std::vector<LogRecord> records_;
};

} // namespace sovereign
