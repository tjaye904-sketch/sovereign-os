#pragma once

#include "event_category.hpp"
#include "event_priority.hpp"
#include "event_security.hpp"
#include "identity_types.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace sovereign
{

struct RuntimeMessageRequest final
{
    std::string request_topic{};
    std::string response_topic{};
    std::string payload{};

    EventCategory category{
        EventCategory::Runtime};

    EventPriority priority{
        EventPriority::Normal};

    EventSecurity security{
        EventSecurity::Internal};

    TransactionId transaction_id{
        TransactionId::Nil()};

    DecisionId decision_id{
        DecisionId::Nil()};

    FacultyId source_faculty{
        FacultyId::Nil()};

    AgentId source_agent{
        AgentId::Nil()};

    std::size_t maximum_process_cycles{1U};
};

enum class RuntimeMessageRequestStatus : std::uint8_t
{
    NoAttempt,
    Completed,
    EmptyRequestTopic,
    EmptyResponseTopic,
    RequestRejected,
    SubscriptionRejected,
    NoResponse
};

struct RuntimeMessageRequestResult final
{
    RuntimeMessageRequestStatus status{
        RuntimeMessageRequestStatus::NoAttempt};

    CorrelationId correlation_id{
        CorrelationId::Nil()};

    EventId request_event_id{
        EventId::Nil()};

    EventId response_event_id{
        EventId::Nil()};

    std::string request_topic{};
    std::string response_topic{};
    std::string response_payload{};

    std::uint64_t request_sequence{0U};
    std::uint64_t response_sequence{0U};

    std::size_t process_cycles{0U};

    [[nodiscard]]
    bool Completed() const noexcept
    {
        return status ==
               RuntimeMessageRequestStatus::Completed;
    }
};

} // namespace sovereign
