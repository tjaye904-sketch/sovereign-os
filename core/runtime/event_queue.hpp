#pragma once

#include "event.hpp"

#include <cstddef>
#include <mutex>
#include <queue>

namespace sovereign
{

class EventQueue final
{
public:
    bool Push(Event event);
    bool Pop(Event& event);

    [[nodiscard]]
    bool Empty() const;

    [[nodiscard]]
    std::size_t Size() const;

private:
    mutable std::mutex mutex_;
    std::queue<Event> events_;
};

} // namespace sovereign
