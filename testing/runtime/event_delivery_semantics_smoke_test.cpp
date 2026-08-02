#include "event_bus.hpp"

#include <cstddef>
#include <optional>
#include <stdexcept>
#include "test_require.hpp"

int main()
{
    sovereign::EventBus bus;

    std::size_t successful_deliveries = 0U;

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.delivery",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy = {},
                .handler =
                    [&successful_deliveries](
                        const sovereign::Event&)
                    {
                        ++successful_deliveries;
                    }}));

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.delivery",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy = {},
                .handler =
                    [](const sovereign::Event&)
                    {
                        throw std::runtime_error{
                            "delivery rejected"};
                    }}));

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.other",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy = {},
                .handler =
                    [](const sovereign::Event&)
                    {
                    }}));

    sovereign::Event event;
    event.topic = "runtime.delivery";

    REQUIRE(bus.Publish(event));

    const auto result = bus.ProcessWithResult();

    REQUIRE(result.evaluated == 3U);
    REQUIRE(result.matched == 2U);
    REQUIRE(result.delivered == 1U);
    REQUIRE(result.rejected == 1U);
    REQUIRE(!result.Complete());

    REQUIRE(successful_deliveries == 1U);
    REQUIRE(bus.PendingCount() == 0U);

    const auto& last = bus.LastDeliveryResult();

    REQUIRE(last.evaluated == result.evaluated);
    REQUIRE(last.matched == result.matched);
    REQUIRE(last.delivered == result.delivered);
    REQUIRE(last.rejected == result.rejected);

    return 0;
}
