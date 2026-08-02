#pragma once

#include "identity_types.hpp"
#include "subscription_configuration.hpp"
#include "subscription_lifecycle.hpp"

#include <cstddef>

namespace sovereign
{

struct SubscriptionSnapshot final
{
    std::size_t index{0U};

    SubscriptionId subscription_id{
        SubscriptionId::Nil()};

    SubscriptionConfiguration configuration{};

    SubscriptionLifecycleSnapshot lifecycle{};

    bool handler_available{false};

    [[nodiscard]]
    constexpr bool Enabled() const noexcept
    {
        return lifecycle.Enabled();
    }
};

} // namespace sovereign
