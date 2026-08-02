#pragma once

#include "health_record.hpp"

#include <cstddef>
#include <mutex>
#include <optional>
#include <string_view>
#include <vector>

namespace sovereign
{

class HealthMonitor final
{
public:
    bool Report(
        HealthRecord record);

    [[nodiscard]]
    std::optional<HealthRecord> Latest(
        std::string_view component) const;

    [[nodiscard]]
    HealthState OverallState() const;

    [[nodiscard]]
    std::size_t RecordCount() const;

    void Clear();

private:
    [[nodiscard]]
    static int Severity(
        HealthState state) noexcept;

    mutable std::mutex mutex_;

    std::vector<HealthRecord> records_;
};

} // namespace sovereign
