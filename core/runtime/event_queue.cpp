#include "event_queue.hpp"

#include <utility>

namespace sovereign
{

bool EventQueue::Push(Event event)
{
    std::scoped_lock lock(mutex_);

    events_.push(std::move(event));

    return true;
}

bool EventQueue::Pop(Event& event)
{
    std::scoped_lock lock(mutex_);

    if (events_.empty())
    {
        return false;
    }

    event = std::move(events_.front());
    events_.pop();

    return true;
}

bool EventQueue::Empty() const
{
    std::scoped_lock lock(mutex_);

    return events_.empty();
}

std::size_t EventQueue::Size() const
{
    std::scoped_lock lock(mutex_);

    return events_.size();
}

} // namespace sovereign
