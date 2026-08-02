#pragma once

#include "event.hpp"
#include "identity_types.hpp"

#include <chrono>
#include <cstddef>
#include <string>

namespace sovereign
{

struct DeadLetterEvent final
{
    Event event;

    SubscriptionId subscription_id{
        SubscriptionId::Nil()};

    std::size_t attempts{0U};

    std::size_t replay_count{0U};

    std::string failure_reason;

    std::chrono::system_clock::time_point timestamp{
        std::chrono::system_clock::now()};
};

} // namespace sovereign
