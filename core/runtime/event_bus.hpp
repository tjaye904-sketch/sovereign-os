#pragma once

#include "dead_letter_queue.hpp"
#include "dead_letter_replay_result.hpp"
#include "event_delivery_result.hpp"
#include "event_queue.hpp"
#include "event_router.hpp"
#include "subscription_configuration.hpp"
#include "subscription_lifecycle.hpp"
#include "subscription_inventory_query.hpp"
#include "subscription_inventory_page.hpp"
#include "subscription_snapshot.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace sovereign
{

class EventBus final
{
public:
    explicit EventBus(
        std::size_t dead_letter_capacity = 1024U) noexcept;

    bool Publish(Event event);

    bool Subscribe(EventSubscription subscription);

    bool Unsubscribe(
        const SubscriptionId& subscription_id) noexcept;

    bool EnableSubscription(
        const SubscriptionId& subscription_id) noexcept;

    bool DisableSubscription(
        const SubscriptionId& subscription_id) noexcept;

    bool ConfigureSubscription(
        const SubscriptionId& subscription_id,
        SubscriptionConfiguration configuration);

    [[nodiscard]]
    std::optional<SubscriptionConfiguration>
    SubscriptionConfigurationInfo(
        const SubscriptionId& subscription_id) const;

    bool Process();

    [[nodiscard]]
    EventDeliveryResult ProcessWithResult();

    [[nodiscard]]
    DeadLetterReplayResult ReplayDeadLetter(
        std::size_t index) noexcept;

    [[nodiscard]]
    const EventDeliveryResult&
    LastDeliveryResult() const noexcept;

    [[nodiscard]]
    const DeadLetterQueue&
    DeadLetters() const noexcept;

    void ClearDeadLetters() noexcept;

    [[nodiscard]]
    std::size_t PendingCount() const;

    [[nodiscard]]
    std::size_t SubscriptionCount() const noexcept;

    [[nodiscard]]
    std::optional<SubscriptionId>
    SubscriptionIdentity(
        std::size_t index) const noexcept;

    [[nodiscard]]
    std::optional<SubscriptionLifecycleSnapshot>
    SubscriptionLifecycleInfo(
        const SubscriptionId& subscription_id) const noexcept;

    [[nodiscard]]
    std::optional<SubscriptionSnapshot>
    SubscriptionSnapshotInfo(
        const SubscriptionId& subscription_id) const;

    [[nodiscard]]
    std::optional<SubscriptionSnapshot>
    SubscriptionSnapshotAt(
        std::size_t index) const;

    [[nodiscard]]
    std::vector<SubscriptionSnapshot>
    SubscriptionInventory() const;

    [[nodiscard]]
    std::vector<SubscriptionSnapshot>
    QuerySubscriptionInventory(
        const SubscriptionInventoryQuery& query) const;

    [[nodiscard]]
    SubscriptionInventoryPage
    QuerySubscriptionInventoryPage(
        const SubscriptionInventoryPageRequest&
            request) const;

    [[nodiscard]]
    bool HasSubscription(
        const SubscriptionId& subscription_id) const noexcept;

    [[nodiscard]]
    bool SubscriptionEnabled(
        const SubscriptionId& subscription_id) const noexcept;

private:
    EventQueue queue_;
    EventRouter router_;
    EventDeliveryResult last_delivery_result_;
};

} // namespace sovereign
