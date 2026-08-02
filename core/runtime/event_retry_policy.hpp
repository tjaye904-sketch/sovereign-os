#pragma once

#include <cstddef>

namespace sovereign
{

struct EventRetryPolicy final
{
    std::size_t maximum_attempts{1};

    [[nodiscard]]
    constexpr std::size_t NormalizedAttempts() const noexcept
    {
        return maximum_attempts == 0U
            ? 1U
            : maximum_attempts;
    }
};

} // namespace sovereign
