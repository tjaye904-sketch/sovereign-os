#include "event_bus.hpp"
#include "scheduler.hpp"

#include <chrono>
#include <cstdint>
#include "test_require.hpp"

int main()
{
    sovereign::EventBus event_bus;
    sovereign::Scheduler scheduler(event_bus);

    std::uint64_t updates = 0;

    scheduler.SetFrameInterval(
        std::chrono::milliseconds(1));

    scheduler.SetMaximumFrames(3);

    scheduler.SetUpdateHandler(
        [&updates](
            const sovereign::FrameContext& context)
        {
            REQUIRE(context.frame_number == updates);
            ++updates;
        });

    scheduler.Run();

    REQUIRE(!scheduler.Running());
    REQUIRE(scheduler.FrameCount() == 3);
    REQUIRE(updates == 3);

    return 0;
}
