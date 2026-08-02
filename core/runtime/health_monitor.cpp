#include "health_monitor.hpp"

#include <utility>

namespace sovereign
{

bool HealthMonitor::Report(
    HealthRecord record)
{
    if (record.component.empty())
    {
        return false;
    }

    std::scoped_lock lock(mutex_);

    records_.push_back(std::move(record));

    return true;
}

std::optional<HealthRecord>
HealthMonitor::Latest(
    std::string_view component) const
{
    std::scoped_lock lock(mutex_);

    for (auto iterator = records_.rbegin();
         iterator != records_.rend();
         ++iterator)
    {
        if (iterator->component == component)
        {
            return *iterator;
        }
    }

    return std::nullopt;
}

HealthState HealthMonitor::OverallState() const
{
    std::scoped_lock lock(mutex_);

    HealthState overall = HealthState::Unknown;
    int highest_severity = Severity(overall);

    for (const auto& record : records_)
    {
        const int severity =
            Severity(record.state);

        if (severity > highest_severity)
        {
            highest_severity = severity;
            overall = record.state;
        }
    }

    return overall;
}

std::size_t HealthMonitor::RecordCount() const
{
    std::scoped_lock lock(mutex_);

    return records_.size();
}

void HealthMonitor::Clear()
{
    std::scoped_lock lock(mutex_);

    records_.clear();
}

int HealthMonitor::Severity(
    HealthState state) noexcept
{
    switch (state)
    {
        case HealthState::Unknown:
            return 0;

        case HealthState::Healthy:
            return 1;

        case HealthState::Degraded:
            return 2;

        case HealthState::Unhealthy:
            return 3;

        case HealthState::Critical:
            return 4;
    }

    return 0;
}

} // namespace sovereign
