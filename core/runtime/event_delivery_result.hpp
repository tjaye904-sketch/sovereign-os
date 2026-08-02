#pragma once

#include <cstddef>

namespace sovereign
{

struct EventDeliveryResult final
{
    std::size_t evaluated{0};
    std::size_t matched{0};
    std::size_t attempts{0};
    std::size_t retried{0};
    std::size_t delivered{0};
    std::size_t rejected{0};
    std::size_t exhausted{0};

    [[nodiscard]]
    constexpr bool Complete() const noexcept
    {
        return delivered == matched;
    }
};

} // namespace sovereign
