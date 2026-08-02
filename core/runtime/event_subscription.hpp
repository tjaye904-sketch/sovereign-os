#pragma once

#include "event.hpp"
#include "event_retry_policy.hpp"
#include "identity_types.hpp"
#include "subscription_configuration.hpp"
#include "subscription_lifecycle.hpp"
#include "subscription_snapshot.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace sovereign
{

using EventHandler = std::function<void(const Event&)>;

struct EventSubscription final
{
    SubscriptionId subscription_id{
        SubscriptionId::Nil()};

    std::string topic;

    std::optional<EventCategory> category;

    std::optional<EventPriority> minimum_priority;

    EventSecurity security_clearance{
        EventSecurity::Internal};

    EventRetryPolicy retry_policy{};

    EventHandler handler;

    SubscriptionLifecycle lifecycle{};

    [[nodiscard]]
    bool Enabled() const noexcept
    {
        return lifecycle.Enabled();
    }

    [[nodiscard]]
    SubscriptionConfiguration Configuration() const
    {
        return SubscriptionConfiguration{
            .topic = topic,
            .category = category,
            .minimum_priority = minimum_priority,
            .security_clearance = security_clearance,
            .retry_policy = retry_policy};
    }

    [[nodiscard]]
    SubscriptionSnapshot Snapshot(
        const std::size_t index) const
    {
        return SubscriptionSnapshot{
            .index = index,
            .subscription_id = subscription_id,
            .configuration = Configuration(),
            .lifecycle =
                SubscriptionLifecycleSnapshot{
                    .subscription_id =
                        subscription_id,
                    .state = lifecycle.state,
                    .created_at =
                        lifecycle.created_at,
                    .updated_at =
                        lifecycle.updated_at,
                    .revision =
                        lifecycle.revision},
            .handler_available =
                static_cast<bool>(handler)};
    }

    bool ApplyConfiguration(
        SubscriptionConfiguration configuration)
    {
        if (Configuration() == configuration)
        {
            return false;
        }

        topic = std::move(configuration.topic);
        category = configuration.category;
        minimum_priority =
            configuration.minimum_priority;
        security_clearance =
            configuration.security_clearance;
        retry_policy = configuration.retry_policy;

        static_cast<void>(
            lifecycle.RecordChange());

        return true;
    }

    [[nodiscard]]
    bool Matches(const Event& event) const noexcept
    {
        if (!Enabled())
        {
            return false;
        }

        if (!topic.empty() &&
            topic != event.topic)
        {
            return false;
        }

        if (category.has_value() &&
            event.metadata.category != *category)
        {
            return false;
        }

        if (minimum_priority.has_value() &&
            PriorityRank(event.metadata.priority) <
                PriorityRank(*minimum_priority))
        {
            return false;
        }

        if (SecurityRank(event.metadata.security) >
            SecurityRank(security_clearance))
        {
            return false;
        }

        return true;
    }

private:
    [[nodiscard]]
    static constexpr std::uint8_t PriorityRank(
        EventPriority priority) noexcept
    {
        return static_cast<std::uint8_t>(priority);
    }

    [[nodiscard]]
    static constexpr std::uint8_t SecurityRank(
        EventSecurity security) noexcept
    {
        return static_cast<std::uint8_t>(security);
    }
};

} // namespace sovereign
