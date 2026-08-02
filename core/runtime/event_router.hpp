#pragma once

#include "dead_letter_queue.hpp"
#include "dead_letter_replay_result.hpp"
#include "event_delivery_result.hpp"
#include "event_subscription.hpp"
#include "subscription_configuration.hpp"
#include "subscription_lifecycle.hpp"
#include "subscription_inventory_query.hpp"
#include "subscription_inventory_page.hpp"
#include "subscription_snapshot.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace sovereign
{

class EventRouter final
{
public:
    explicit EventRouter(
        std::size_t dead_letter_capacity = 1024U) noexcept;

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

    [[nodiscard]]
    EventDeliveryResult Route(
        const Event& event) noexcept;

    [[nodiscard]]
    DeadLetterReplayResult ReplayDeadLetter(
        std::size_t index) noexcept;

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

    [[nodiscard]]
    const DeadLetterQueue&
    DeadLetters() const noexcept;

    void ClearDeadLetters() noexcept;

private:
    [[nodiscard]]
    std::optional<std::size_t>
    FindSubscriptionIndex(
        const SubscriptionId& subscription_id) const noexcept;

    bool SetSubscriptionEnabled(
        const SubscriptionId& subscription_id,
        bool enabled) noexcept;

    [[nodiscard]]
    EventDeliveryResult Deliver(
        const Event& event,
        std::size_t subscription_index,
        std::string& failure_reason) const noexcept;

    std::vector<EventSubscription> subscriptions_;
    DeadLetterQueue dead_letters_;
};

} // namespace sovereign
