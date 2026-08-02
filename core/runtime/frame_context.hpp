#pragma once

#include <chrono>
#include <cstdint>

namespace sovereign
{

struct FrameContext final
{
    std::uint64_t frame_number{0};

    std::chrono::steady_clock::duration delta_time{};

    std::chrono::steady_clock::duration elapsed_time{};
};

} // namespace sovereign
