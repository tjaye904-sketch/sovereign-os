#pragma once

#include "event_delivery_result.hpp"
#include "identity_types.hpp"

#include <cstddef>

namespace sovereign
{

struct DeadLetterReplayResult final
{
    bool found{false};
    bool target_found{false};
    bool delivered{false};
    bool retained{false};

    std::size_t dead_letter_index{0U};

    SubscriptionId subscription_id{
        SubscriptionId::Nil()};

    std::size_t replay_count{0U};

    EventDeliveryResult delivery{};

    [[nodiscard]]
    constexpr bool Complete() const noexcept
    {
        return found &&
               target_found &&
               delivered &&
               !retained;
    }
};

} // namespace sovereign
