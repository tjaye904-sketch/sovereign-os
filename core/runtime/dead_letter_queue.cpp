#include "dead_letter_queue.hpp"

#include <utility>

namespace sovereign
{

DeadLetterQueue::DeadLetterQueue(
    const std::size_t capacity) noexcept
    : capacity_{capacity}
{
}

bool DeadLetterQueue::Push(DeadLetterEvent event)
{
    if (capacity_ == 0U)
    {
        ++discarded_count_;
        return false;
    }

    if (entries_.size() == capacity_)
    {
        entries_.pop_front();
        ++discarded_count_;
    }

    entries_.push_back(std::move(event));

    return true;
}

bool DeadLetterQueue::Replace(
    const std::size_t index,
    DeadLetterEvent event)
{
    if (index >= entries_.size())
    {
        return false;
    }

    entries_[index] = std::move(event);

    return true;
}

bool DeadLetterQueue::Erase(
    const std::size_t index) noexcept
{
    if (index >= entries_.size())
    {
        return false;
    }

    const auto offset =
        static_cast<std::deque<DeadLetterEvent>::
            difference_type>(index);

    entries_.erase(entries_.begin() + offset);

    return true;
}

const DeadLetterEvent*
DeadLetterQueue::Find(
    const std::size_t index) const noexcept
{
    if (index >= entries_.size())
    {
        return nullptr;
    }

    return &entries_[index];
}

bool DeadLetterQueue::Empty() const noexcept
{
    return entries_.empty();
}

std::size_t DeadLetterQueue::Size() const noexcept
{
    return entries_.size();
}

std::size_t DeadLetterQueue::Capacity() const noexcept
{
    return capacity_;
}

std::size_t DeadLetterQueue::DiscardedCount() const noexcept
{
    return discarded_count_;
}

const std::deque<DeadLetterEvent>&
DeadLetterQueue::Entries() const noexcept
{
    return entries_;
}

void DeadLetterQueue::Clear() noexcept
{
    entries_.clear();
}

} // namespace sovereign
