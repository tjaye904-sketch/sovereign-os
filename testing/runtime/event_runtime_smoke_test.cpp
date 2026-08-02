#include "event_bus.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include "test_require.hpp"

int main()
{
    sovereign::EventBus bus;

    std::size_t received_count = 0;
    std::string received_payload;

    const bool subscribed =
        bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.started",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy = {},
                .handler =
                    [&received_count, &received_payload](
                        const sovereign::Event& event)
                    {
                        ++received_count;
                        received_payload = event.payload;
                    }});

    REQUIRE(subscribed);
    REQUIRE(bus.SubscriptionCount() == 1U);

    sovereign::Event event;
    event.metadata.sequence = 1U;
    event.topic = "runtime.started";
    event.payload = "Sovereign event runtime online.";

    REQUIRE(bus.Publish(event));
    REQUIRE(bus.PendingCount() == 1U);
    REQUIRE(bus.Process());
    REQUIRE(bus.PendingCount() == 0U);
    REQUIRE(received_count == 1U);
    REQUIRE(received_payload == "Sovereign event runtime online.");

    return 0;
}
