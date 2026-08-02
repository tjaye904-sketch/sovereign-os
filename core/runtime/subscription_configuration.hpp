#pragma once

#include "event_category.hpp"
#include "event_priority.hpp"
#include "event_retry_policy.hpp"
#include "event_security.hpp"

#include <optional>
#include <string>

namespace sovereign
{

struct SubscriptionConfiguration final
{
    std::string topic;

    std::optional<EventCategory> category;

    std::optional<EventPriority> minimum_priority;

    EventSecurity security_clearance{
        EventSecurity::Internal};

    EventRetryPolicy retry_policy{};

    [[nodiscard]]
    bool operator==(
        const SubscriptionConfiguration& other) const noexcept
    {
        return topic == other.topic &&
               category == other.category &&
               minimum_priority ==
                   other.minimum_priority &&
               security_clearance ==
                   other.security_clearance &&
               retry_policy.maximum_attempts ==
                   other.retry_policy.maximum_attempts;
    }
};

} // namespace sovereign
