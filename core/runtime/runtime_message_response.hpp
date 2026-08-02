#pragma once

#include "event.hpp"
#include "event_category.hpp"
#include "event_priority.hpp"
#include "event_security.hpp"
#include "identity_types.hpp"

#include <optional>
#include <string>

namespace sovereign
{

struct RuntimeMessageResponse final
{
    std::string topic{};
    std::string payload{};

    std::optional<EventCategory> category{};
    std::optional<EventPriority> priority{};
    std::optional<EventSecurity> security{};

    std::optional<TransactionId> transaction_id{};
    std::optional<DecisionId> decision_id{};

    std::optional<FacultyId> source_faculty{};
    std::optional<AgentId> source_agent{};
};

} // namespace sovereign
