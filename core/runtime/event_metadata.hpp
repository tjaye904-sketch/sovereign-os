#pragma once

#include "identity_types.hpp"
#include "event_security.hpp"
#include "event_priority.hpp"
#include "event_category.hpp"

#include <chrono>
#include <cstdint>

namespace sovereign
{

struct EventMetadata final
{
    EventCategory category{EventCategory::Runtime};

    EventPriority priority{EventPriority::Normal};

    EventSecurity security{EventSecurity::Internal};

    EventId event_id{EventId::Generate()};

    CorrelationId correlation_id{
        CorrelationId::Nil()};

    TransactionId transaction_id{
        TransactionId::Nil()};

    DecisionId decision_id{
        DecisionId::Nil()};

    RuntimeId source_runtime{
        RuntimeId::Nil()};

    NodeId source_node{
        NodeId::Nil()};

    FacultyId source_faculty{
        FacultyId::Nil()};

    AgentId source_agent{
        AgentId::Nil()};

    std::chrono::system_clock::time_point created{
        std::chrono::system_clock::now()};

    std::uint64_t sequence{0};
};

} // namespace sovereign
