#pragma once

#include "event_bus.hpp"
#include "frame_context.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>

namespace sovereign
{

class Scheduler final
{
public:
    using UpdateHandler =
        std::function<void(const FrameContext&)>;

    explicit Scheduler(EventBus& event_bus);

    void SetUpdateHandler(UpdateHandler handler);

    void SetFrameInterval(
        std::chrono::steady_clock::duration interval);

    void SetMaximumFrames(
        std::uint64_t maximum_frames) noexcept;

    void Run();

    void Stop() noexcept;

    [[nodiscard]]
    bool Running() const noexcept;

    [[nodiscard]]
    std::uint64_t FrameCount() const noexcept;

private:
    EventBus& event_bus_;

    UpdateHandler update_handler_;

    std::chrono::steady_clock::duration frame_interval_{
        std::chrono::milliseconds(16)};

    std::atomic<bool> running_{false};

    std::uint64_t maximum_frames_{0};

    std::uint64_t frame_count_{0};
};

} // namespace sovereign
