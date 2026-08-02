#include "scheduler.hpp"

#include <thread>
#include <utility>

namespace sovereign
{

Scheduler::Scheduler(EventBus& event_bus)
    :
    event_bus_(event_bus)
{
}

void Scheduler::SetUpdateHandler(
    UpdateHandler handler)
{
    update_handler_ = std::move(handler);
}

void Scheduler::SetFrameInterval(
    std::chrono::steady_clock::duration interval)
{
    frame_interval_ = interval;
}

void Scheduler::SetMaximumFrames(
    std::uint64_t maximum_frames) noexcept
{
    maximum_frames_ = maximum_frames;
}

void Scheduler::Run()
{
    running_.store(
        true,
        std::memory_order_release);

    frame_count_ = 0;

    const auto start_time =
        std::chrono::steady_clock::now();

    auto previous_frame_time =
        start_time;

    while (running_.load(
        std::memory_order_acquire))
    {
        const auto frame_begin =
            std::chrono::steady_clock::now();

        FrameContext context;
        context.frame_number = frame_count_;
        context.delta_time =
            frame_begin - previous_frame_time;
        context.elapsed_time =
            frame_begin - start_time;

        previous_frame_time =
            frame_begin;

        event_bus_.Process();

        if (update_handler_)
        {
            update_handler_(context);
        }

        ++frame_count_;

        if (maximum_frames_ != 0 &&
            frame_count_ >= maximum_frames_)
        {
            Stop();
            continue;
        }

        const auto frame_end =
            std::chrono::steady_clock::now();

        const auto execution_time =
            frame_end - frame_begin;

        if (execution_time < frame_interval_)
        {
            std::this_thread::sleep_for(
                frame_interval_ - execution_time);
        }
    }
}

void Scheduler::Stop() noexcept
{
    running_.store(
        false,
        std::memory_order_release);
}

bool Scheduler::Running() const noexcept
{
    return running_.load(
        std::memory_order_acquire);
}

std::uint64_t Scheduler::FrameCount() const noexcept
{
    return frame_count_;
}

} // namespace sovereign
