#pragma once

#include "identity_types.hpp"

#include <chrono>
#include <cstddef>

namespace sovereign
{

enum class SubscriptionState
{
    Enabled,
    Disabled
};

struct SubscriptionLifecycle final
{
    SubscriptionState state{
        SubscriptionState::Enabled};

    std::chrono::system_clock::time_point created_at{
        std::chrono::system_clock::now()};

    std::chrono::system_clock::time_point updated_at{
        created_at};

    std::size_t revision{0U};

    [[nodiscard]]
    constexpr bool Enabled() const noexcept
    {
        return state == SubscriptionState::Enabled;
    }

    bool RecordChange() noexcept
    {
        updated_at =
            std::chrono::system_clock::now();

        ++revision;

        return true;
    }

    bool SetEnabled(const bool enabled) noexcept
    {
        const auto requested_state =
            enabled
                ? SubscriptionState::Enabled
                : SubscriptionState::Disabled;

        if (state == requested_state)
        {
            return false;
        }

        state = requested_state;
        updated_at =
            std::chrono::system_clock::now();

        ++revision;

        return true;
    }
};

struct SubscriptionLifecycleSnapshot final
{
    SubscriptionId subscription_id{
        SubscriptionId::Nil()};

    SubscriptionState state{
        SubscriptionState::Enabled};

    std::chrono::system_clock::time_point created_at{};

    std::chrono::system_clock::time_point updated_at{};

    std::size_t revision{0U};

    [[nodiscard]]
    constexpr bool Enabled() const noexcept
    {
        return state == SubscriptionState::Enabled;
    }
};

} // namespace sovereign
