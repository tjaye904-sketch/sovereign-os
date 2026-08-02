#include "event_router.hpp"

#include <algorithm>
#include <chrono>
#include <exception>
#include <string>
#include <utility>

namespace sovereign
{

EventRouter::EventRouter(
    const std::size_t dead_letter_capacity) noexcept
    : dead_letters_{dead_letter_capacity}
{
}

bool EventRouter::Subscribe(
    EventSubscription subscription)
{
    if (!subscription.handler)
    {
        return false;
    }

    if (subscription.subscription_id.IsNil())
    {
        subscription.subscription_id =
            SubscriptionId::Generate();
    }

    if (HasSubscription(
            subscription.subscription_id))
    {
        return false;
    }

    const auto now =
        std::chrono::system_clock::now();

    subscription.lifecycle.created_at = now;
    subscription.lifecycle.updated_at = now;
    subscription.lifecycle.revision = 0U;

    subscriptions_.push_back(
        std::move(subscription));

    return true;
}

bool EventRouter::Unsubscribe(
    const SubscriptionId& subscription_id) noexcept
{
    if (subscription_id.IsNil())
    {
        return false;
    }

    const auto iterator =
        std::find_if(
            subscriptions_.begin(),
            subscriptions_.end(),
            [&subscription_id](
                const EventSubscription& subscription)
            {
                return subscription.subscription_id ==
                       subscription_id;
            });

    if (iterator == subscriptions_.end())
    {
        return false;
    }

    subscriptions_.erase(iterator);

    return true;
}

std::optional<std::size_t>
EventRouter::FindSubscriptionIndex(
    const SubscriptionId& subscription_id) const noexcept
{
    if (subscription_id.IsNil())
    {
        return std::nullopt;
    }

    for (std::size_t index = 0U;
         index < subscriptions_.size();
         ++index)
    {
        if (subscriptions_[index].subscription_id ==
            subscription_id)
        {
            return index;
        }
    }

    return std::nullopt;
}

bool EventRouter::SetSubscriptionEnabled(
    const SubscriptionId& subscription_id,
    const bool enabled) noexcept
{
    const auto index =
        FindSubscriptionIndex(subscription_id);

    if (!index.has_value())
    {
        return false;
    }

    /*
     * The operation is intentionally idempotent. Finding the
     * subscription means the requested lifecycle operation was
     * accepted, even when it was already in the requested state.
     */
    static_cast<void>(
        subscriptions_[*index]
            .lifecycle
            .SetEnabled(enabled));

    return true;
}

bool EventRouter::EnableSubscription(
    const SubscriptionId& subscription_id) noexcept
{
    return SetSubscriptionEnabled(
        subscription_id,
        true);
}

bool EventRouter::DisableSubscription(
    const SubscriptionId& subscription_id) noexcept
{
    return SetSubscriptionEnabled(
        subscription_id,
        false);
}

bool EventRouter::ConfigureSubscription(
    const SubscriptionId& subscription_id,
    SubscriptionConfiguration configuration)
{
    const auto index =
        FindSubscriptionIndex(subscription_id);

    if (!index.has_value())
    {
        return false;
    }

    static_cast<void>(
        subscriptions_[*index]
            .ApplyConfiguration(
                std::move(configuration)));

    /*
     * Configuration is intentionally idempotent. A valid
     * subscription accepts an identical configuration without
     * incrementing its lifecycle revision.
     */
    return true;
}

std::optional<SubscriptionConfiguration>
EventRouter::SubscriptionConfigurationInfo(
    const SubscriptionId& subscription_id) const
{
    const auto index =
        FindSubscriptionIndex(subscription_id);

    if (!index.has_value())
    {
        return std::nullopt;
    }

    return subscriptions_[*index].Configuration();
}

bool EventRouter::HasSubscription(
    const SubscriptionId& subscription_id) const noexcept
{
    return FindSubscriptionIndex(
               subscription_id)
        .has_value();
}

bool EventRouter::SubscriptionEnabled(
    const SubscriptionId& subscription_id) const noexcept
{
    const auto index =
        FindSubscriptionIndex(subscription_id);

    if (!index.has_value())
    {
        return false;
    }

    return subscriptions_[*index].Enabled();
}

std::optional<SubscriptionLifecycleSnapshot>
EventRouter::SubscriptionLifecycleInfo(
    const SubscriptionId& subscription_id) const noexcept
{
    const auto index =
        FindSubscriptionIndex(subscription_id);

    if (!index.has_value())
    {
        return std::nullopt;
    }

    const auto& subscription =
        subscriptions_[*index];

    return SubscriptionLifecycleSnapshot{
        .subscription_id =
            subscription.subscription_id,
        .state =
            subscription.lifecycle.state,
        .created_at =
            subscription.lifecycle.created_at,
        .updated_at =
            subscription.lifecycle.updated_at,
        .revision =
            subscription.lifecycle.revision};
}

std::optional<SubscriptionSnapshot>
EventRouter::SubscriptionSnapshotInfo(
    const SubscriptionId& subscription_id) const
{
    const auto index =
        FindSubscriptionIndex(subscription_id);

    if (!index.has_value())
    {
        return std::nullopt;
    }

    return subscriptions_[*index].Snapshot(*index);
}

std::optional<SubscriptionSnapshot>
EventRouter::SubscriptionSnapshotAt(
    const std::size_t index) const
{
    if (index >= subscriptions_.size())
    {
        return std::nullopt;
    }

    return subscriptions_[index].Snapshot(index);
}

std::vector<SubscriptionSnapshot>
EventRouter::SubscriptionInventory() const
{
    std::vector<SubscriptionSnapshot> inventory;
    inventory.reserve(subscriptions_.size());

    for (std::size_t index = 0U;
         index < subscriptions_.size();
         ++index)
    {
        inventory.push_back(
            subscriptions_[index].Snapshot(index));
    }

    return inventory;
}

std::vector<SubscriptionSnapshot>
EventRouter::QuerySubscriptionInventory(
    const SubscriptionInventoryQuery& query) const
{
    std::vector<SubscriptionSnapshot> matches;
    matches.reserve(subscriptions_.size());

    for (std::size_t index = 0U;
         index < subscriptions_.size();
         ++index)
    {
        const SubscriptionSnapshot snapshot =
            subscriptions_[index].Snapshot(index);

        if (query.topic.has_value() &&
            snapshot.configuration.topic != *query.topic)
        {
            continue;
        }

        if (query.category.has_value() &&
            snapshot.configuration.category != query.category)
        {
            continue;
        }

        if (query.minimum_priority.has_value() &&
            snapshot.configuration.minimum_priority !=
                query.minimum_priority)
        {
            continue;
        }

        if (query.security_clearance.has_value() &&
            snapshot.configuration.security_clearance !=
                *query.security_clearance)
        {
            continue;
        }

        if (query.enabled.has_value() &&
            snapshot.Enabled() != *query.enabled)
        {
            continue;
        }

        if (query.handler_available.has_value() &&
            snapshot.handler_available !=
                *query.handler_available)
        {
            continue;
        }

        matches.push_back(snapshot);
    }

    return matches;
}

SubscriptionInventoryPage
EventRouter::QuerySubscriptionInventoryPage(
    const SubscriptionInventoryPageRequest& request) const
{
    std::vector<SubscriptionSnapshot> matches =
        QuerySubscriptionInventory(request.query);

    const auto compare_field =
        [](
            const SubscriptionSnapshot& left,
            const SubscriptionSnapshot& right,
            const SubscriptionInventorySortField field)
        {
            switch (field)
            {
                case SubscriptionInventorySortField::
                    InventoryIndex:
                    if (left.index < right.index)
                    {
                        return -1;
                    }

                    if (right.index < left.index)
                    {
                        return 1;
                    }

                    return 0;

                case SubscriptionInventorySortField::Topic:
                    if (left.configuration.topic <
                        right.configuration.topic)
                    {
                        return -1;
                    }

                    if (right.configuration.topic <
                        left.configuration.topic)
                    {
                        return 1;
                    }

                    return 0;

                case SubscriptionInventorySortField::Category:
                    if (left.configuration.category <
                        right.configuration.category)
                    {
                        return -1;
                    }

                    if (right.configuration.category <
                        left.configuration.category)
                    {
                        return 1;
                    }

                    return 0;

                case SubscriptionInventorySortField::
                    MinimumPriority:
                    if (left.configuration.minimum_priority <
                        right.configuration.minimum_priority)
                    {
                        return -1;
                    }

                    if (right.configuration.minimum_priority <
                        left.configuration.minimum_priority)
                    {
                        return 1;
                    }

                    return 0;

                case SubscriptionInventorySortField::
                    SecurityClearance:
                    if (left.configuration.security_clearance <
                        right.configuration.security_clearance)
                    {
                        return -1;
                    }

                    if (right.configuration.security_clearance <
                        left.configuration.security_clearance)
                    {
                        return 1;
                    }

                    return 0;

                case SubscriptionInventorySortField::Enabled:
                    if (left.Enabled() < right.Enabled())
                    {
                        return -1;
                    }

                    if (right.Enabled() < left.Enabled())
                    {
                        return 1;
                    }

                    return 0;

                case SubscriptionInventorySortField::
                    HandlerAvailable:
                    if (left.handler_available <
                        right.handler_available)
                    {
                        return -1;
                    }

                    if (right.handler_available <
                        left.handler_available)
                    {
                        return 1;
                    }

                    return 0;
            }

            return 0;
        };

    const auto compare_term =
        [&compare_field](
            const SubscriptionSnapshot& left,
            const SubscriptionSnapshot& right,
            const SubscriptionInventorySortTerm& term)
        {
            const int comparison =
                compare_field(
                    left,
                    right,
                    term.field);

            if (term.direction ==
                SubscriptionInventorySortDirection::Descending)
            {
                return -comparison;
            }

            return comparison;
        };

    const NormalizedSubscriptionInventorySort
        normalized_sort =
            NormalizeSubscriptionInventorySort(
                request.sort);

    std::sort(
        matches.begin(),
        matches.end(),
        [&normalized_sort, &compare_term](
            const SubscriptionSnapshot& left,
            const SubscriptionSnapshot& right)
        {
            for (const SubscriptionInventorySortTerm& term :
                 normalized_sort.terms)
            {
                const int comparison =
                    compare_term(
                        left,
                        right,
                        term);

                if (comparison != 0)
                {
                    return comparison < 0;
                }
            }

            return left.index < right.index;
        });

    const std::size_t active_query_filters =
        static_cast<std::size_t>(
            request.query.topic.has_value()) +
        static_cast<std::size_t>(
            request.query.category.has_value()) +
        static_cast<std::size_t>(
            request.query.minimum_priority.has_value()) +
        static_cast<std::size_t>(
            request.query.security_clearance.has_value()) +
        static_cast<std::size_t>(
            request.query.enabled.has_value()) +
        static_cast<std::size_t>(
            request.query.handler_available.has_value());

    SubscriptionInventoryPage page{
        .entries = {},
        .offset = request.offset,
        .limit = request.limit,
        .total_matches = matches.size(),
        .has_more = false,
        .sort_diagnostics =
            SubscriptionInventorySortDiagnostics{
                .effective_terms =
                    normalized_sort.terms,
                .ignored_terms =
                    normalized_sort.ignored_terms,
                .used_legacy_fields =
                    normalized_sort.used_legacy_fields,
                .used_fallback =
                    normalized_sort.used_fallback},
        .query_diagnostics =
            SubscriptionInventoryQueryDiagnostics{
                .effective_query = request.query,
                .active_filters =
                    active_query_filters,
                .subscriptions_scanned =
                    subscriptions_.size(),
                .subscriptions_matched =
                    matches.size()},
        .execution_diagnostics =
            SubscriptionInventoryExecutionDiagnostics{
                .requested_offset =
                    request.offset,
                .requested_limit =
                    request.limit,
                .matched_entries =
                    matches.size(),
                .returned_entries = 0U,
                .remaining_entries =
                    request.offset < matches.size()
                        ? matches.size() -
                              request.offset
                        : 0U,
                .offset_in_range =
                    request.offset < matches.size(),
                .metadata_only =
                    request.limit == 0U,
                .terminal_page =
                    request.offset >=
                    matches.size()}};

    if (request.limit == 0U ||
        request.offset >= matches.size())
    {
        return page;
    }

    const std::size_t available =
        matches.size() - request.offset;

    const std::size_t result_count =
        request.limit < available
            ? request.limit
            : available;

    page.entries.reserve(result_count);

    for (std::size_t index = 0U;
         index < result_count;
         ++index)
    {
        page.entries.push_back(
            matches[request.offset + index]);
    }

    page.has_more =
        result_count < available;

    page.execution_diagnostics.returned_entries =
        result_count;

    page.execution_diagnostics.remaining_entries =
        available - result_count;

    page.execution_diagnostics.terminal_page =
        !page.has_more;

    return page;
}

EventDeliveryResult EventRouter::Deliver(
    const Event& event,
    const std::size_t subscription_index,
    std::string& failure_reason) const noexcept
{
    EventDeliveryResult result;

    if (subscription_index >= subscriptions_.size())
    {
        return result;
    }

    const auto& subscription =
        subscriptions_[subscription_index];

    if (!subscription.Enabled())
    {
        failure_reason =
            "subscription target disabled";

        return result;
    }

    ++result.evaluated;

    if (!subscription.Matches(event))
    {
        return result;
    }

    ++result.matched;

    const std::size_t maximum_attempts =
        subscription.retry_policy.NormalizedAttempts();

    for (std::size_t attempt = 0U;
         attempt < maximum_attempts;
         ++attempt)
    {
        ++result.attempts;

        if (attempt > 0U)
        {
            ++result.retried;
        }

        try
        {
            subscription.handler(event);
            ++result.delivered;
            return result;
        }
        catch (const std::exception& exception)
        {
            ++result.rejected;
            failure_reason = exception.what();
        }
        catch (...)
        {
            ++result.rejected;
            failure_reason =
                "handler threw a non-standard exception";
        }
    }

    ++result.exhausted;

    return result;
}

EventDeliveryResult EventRouter::Route(
    const Event& event) noexcept
{
    EventDeliveryResult aggregate;

    for (std::size_t index = 0U;
         index < subscriptions_.size();
         ++index)
    {
        if (!subscriptions_[index].Enabled())
        {
            continue;
        }

        std::string failure_reason{
            "handler rejected delivery"};

        const auto result =
            Deliver(event, index, failure_reason);

        aggregate.evaluated += result.evaluated;
        aggregate.matched += result.matched;
        aggregate.attempts += result.attempts;
        aggregate.retried += result.retried;
        aggregate.delivered += result.delivered;
        aggregate.rejected += result.rejected;
        aggregate.exhausted += result.exhausted;

        if (result.exhausted != 0U)
        {
            static_cast<void>(
                dead_letters_.Push(
                    DeadLetterEvent{
                        .event = event,
                        .subscription_id =
                            subscriptions_[index]
                                .subscription_id,
                        .attempts = result.attempts,
                        .replay_count = 0U,
                        .failure_reason =
                            std::move(failure_reason),
                        .timestamp =
                            std::chrono::system_clock::now()}));
        }
    }

    return aggregate;
}

DeadLetterReplayResult EventRouter::ReplayDeadLetter(
    const std::size_t index) noexcept
{
    DeadLetterReplayResult replay_result;
    replay_result.dead_letter_index = index;

    const DeadLetterEvent* stored =
        dead_letters_.Find(index);

    if (stored == nullptr)
    {
        return replay_result;
    }

    replay_result.found = true;

    DeadLetterEvent replayed = *stored;

    replay_result.subscription_id =
        replayed.subscription_id;

    ++replayed.replay_count;

    replay_result.replay_count =
        replayed.replay_count;

    const auto subscription_index =
        FindSubscriptionIndex(
            replayed.subscription_id);

    if (!subscription_index.has_value())
    {
        replayed.failure_reason =
            "subscription target unavailable";

        replayed.timestamp =
            std::chrono::system_clock::now();

        replay_result.retained =
            dead_letters_.Replace(
                index,
                std::move(replayed));

        return replay_result;
    }

    replay_result.target_found = true;

    if (!subscriptions_[*subscription_index].Enabled())
    {
        replayed.failure_reason =
            "subscription target disabled";

        replayed.timestamp =
            std::chrono::system_clock::now();

        replay_result.retained =
            dead_letters_.Replace(
                index,
                std::move(replayed));

        return replay_result;
    }

    std::string failure_reason{
        "handler rejected replay"};

    replay_result.delivery =
        Deliver(
            replayed.event,
            *subscription_index,
            failure_reason);

    if (replay_result.delivery.delivered != 0U)
    {
        replay_result.delivered = true;

        static_cast<void>(
            dead_letters_.Erase(index));

        return replay_result;
    }

    replayed.attempts +=
        replay_result.delivery.attempts;

    replayed.failure_reason =
        std::move(failure_reason);

    replayed.timestamp =
        std::chrono::system_clock::now();

    replay_result.retained =
        dead_letters_.Replace(
            index,
            std::move(replayed));

    return replay_result;
}

std::size_t EventRouter::SubscriptionCount() const noexcept
{
    return subscriptions_.size();
}

std::optional<SubscriptionId>
EventRouter::SubscriptionIdentity(
    const std::size_t index) const noexcept
{
    if (index >= subscriptions_.size())
    {
        return std::nullopt;
    }

    return subscriptions_[index].subscription_id;
}

const DeadLetterQueue&
EventRouter::DeadLetters() const noexcept
{
    return dead_letters_;
}

void EventRouter::ClearDeadLetters() noexcept
{
    dead_letters_.Clear();
}

} // namespace sovereign
