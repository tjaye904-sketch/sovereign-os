#pragma once

#include "event_category.hpp"
#include "event_priority.hpp"
#include "event_security.hpp"

#include <optional>
#include <string>

namespace sovereign
{

struct SubscriptionInventoryQuery final
{
    std::optional<std::string> topic;

    std::optional<EventCategory> category;

    std::optional<EventPriority> minimum_priority;

    std::optional<EventSecurity> security_clearance;

    std::optional<bool> enabled;

    std::optional<bool> handler_available;
};

} // namespace sovereign
