#pragma once

#include "dead_letter_event.hpp"

#include <cstddef>
#include <deque>

namespace sovereign
{

class DeadLetterQueue final
{
public:
    explicit DeadLetterQueue(
        std::size_t capacity = 1024U) noexcept;

    bool Push(DeadLetterEvent event);

    [[nodiscard]]
    bool Replace(
        std::size_t index,
        DeadLetterEvent event);

    [[nodiscard]]
    bool Erase(std::size_t index) noexcept;

    [[nodiscard]]
    const DeadLetterEvent*
    Find(std::size_t index) const noexcept;

    [[nodiscard]]
    bool Empty() const noexcept;

    [[nodiscard]]
    std::size_t Size() const noexcept;

    [[nodiscard]]
    std::size_t Capacity() const noexcept;

    [[nodiscard]]
    std::size_t DiscardedCount() const noexcept;

    [[nodiscard]]
    const std::deque<DeadLetterEvent>&
    Entries() const noexcept;

    void Clear() noexcept;

private:
    std::size_t capacity_;
    std::size_t discarded_count_{0U};
    std::deque<DeadLetterEvent> entries_;
};

} // namespace sovereign
