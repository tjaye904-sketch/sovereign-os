#include "event_bus.hpp"

#include <utility>

namespace sovereign
{

EventBus::EventBus(
    const std::size_t dead_letter_capacity) noexcept
    : router_{dead_letter_capacity}
{
}

bool EventBus::Publish(Event event)
{
    return queue_.Push(std::move(event));
}

bool EventBus::Subscribe(EventSubscription subscription)
{
    return router_.Subscribe(std::move(subscription));
}

bool EventBus::Unsubscribe(
    const SubscriptionId& subscription_id) noexcept
{
    return router_.Unsubscribe(subscription_id);
}


bool EventBus::Process()
{
    return ProcessWithResult().Complete();
}

EventDeliveryResult EventBus::ProcessWithResult()
{
    EventDeliveryResult aggregate;
    Event event;

    while (queue_.Pop(event))
    {
        const auto result = router_.Route(event);

        aggregate.evaluated += result.evaluated;
        aggregate.matched += result.matched;
        aggregate.attempts += result.attempts;
        aggregate.retried += result.retried;
        aggregate.delivered += result.delivered;
        aggregate.rejected += result.rejected;
        aggregate.exhausted += result.exhausted;
    }

    last_delivery_result_ = aggregate;

    return aggregate;
}

DeadLetterReplayResult EventBus::ReplayDeadLetter(
    const std::size_t index) noexcept
{
    return router_.ReplayDeadLetter(index);
}

const EventDeliveryResult&
EventBus::LastDeliveryResult() const noexcept
{
    return last_delivery_result_;
}

const DeadLetterQueue&
EventBus::DeadLetters() const noexcept
{
    return router_.DeadLetters();
}

void EventBus::ClearDeadLetters() noexcept
{
    router_.ClearDeadLetters();
}

std::size_t EventBus::PendingCount() const
{
    return queue_.Size();
}

std::size_t EventBus::SubscriptionCount() const noexcept
{
    return router_.SubscriptionCount();
}

std::optional<SubscriptionId>
EventBus::SubscriptionIdentity(
    const std::size_t index) const noexcept
{
    return router_.SubscriptionIdentity(index);
}

bool EventBus::HasSubscription(
    const SubscriptionId& subscription_id) const noexcept
{
    return router_.HasSubscription(subscription_id);
}



bool EventBus::EnableSubscription(
    const SubscriptionId& subscription_id) noexcept
{
    return router_.EnableSubscription(subscription_id);
}

bool EventBus::DisableSubscription(
    const SubscriptionId& subscription_id) noexcept
{
    return router_.DisableSubscription(subscription_id);
}

bool EventBus::ConfigureSubscription(
    const SubscriptionId& subscription_id,
    SubscriptionConfiguration configuration)
{
    return router_.ConfigureSubscription(
        subscription_id,
        std::move(configuration));
}

std::optional<SubscriptionConfiguration>
EventBus::SubscriptionConfigurationInfo(
    const SubscriptionId& subscription_id) const
{
    return router_.SubscriptionConfigurationInfo(
        subscription_id);
}

std::optional<SubscriptionLifecycleSnapshot>
EventBus::SubscriptionLifecycleInfo(
    const SubscriptionId& subscription_id) const noexcept
{
    return router_.SubscriptionLifecycleInfo(subscription_id);
}

std::optional<SubscriptionSnapshot>
EventBus::SubscriptionSnapshotInfo(
    const SubscriptionId& subscription_id) const
{
    return router_.SubscriptionSnapshotInfo(
        subscription_id);
}

std::optional<SubscriptionSnapshot>
EventBus::SubscriptionSnapshotAt(
    const std::size_t index) const
{
    return router_.SubscriptionSnapshotAt(index);
}

std::vector<SubscriptionSnapshot>
EventBus::SubscriptionInventory() const
{
    return router_.SubscriptionInventory();
}

std::vector<SubscriptionSnapshot>
EventBus::QuerySubscriptionInventory(
    const SubscriptionInventoryQuery& query) const
{
    return router_.QuerySubscriptionInventory(query);
}

SubscriptionInventoryPage
EventBus::QuerySubscriptionInventoryPage(
    const SubscriptionInventoryPageRequest& request) const
{
    return router_.QuerySubscriptionInventoryPage(request);
}

bool EventBus::SubscriptionEnabled(
    const SubscriptionId& subscription_id) const noexcept
{
    return router_.SubscriptionEnabled(subscription_id);
}

} // namespace sovereign
