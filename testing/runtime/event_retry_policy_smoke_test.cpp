#include "event_bus.hpp"

#include <cstddef>
#include <optional>
#include <stdexcept>
#include "test_require.hpp"

int main()
{
    sovereign::EventBus bus;

    std::size_t transient_attempts = 0U;
    std::size_t permanent_attempts = 0U;

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.retry",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy =
                    sovereign::EventRetryPolicy{
                        .maximum_attempts = 3U},
                .handler =
                    [&transient_attempts](
                        const sovereign::Event&)
                    {
                        ++transient_attempts;

                        if (transient_attempts < 3U)
                        {
                            throw std::runtime_error{
                                "transient failure"};
                        }
                    }}));

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "runtime.retry",
                .category = std::nullopt,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy =
                    sovereign::EventRetryPolicy{
                        .maximum_attempts = 2U},
                .handler =
                    [&permanent_attempts](
                        const sovereign::Event&)
                    {
                        ++permanent_attempts;

                        throw std::runtime_error{
                            "permanent failure"};
                    }}));

    sovereign::Event event;
    event.topic = "runtime.retry";

    REQUIRE(bus.Publish(event));

    const auto result = bus.ProcessWithResult();

    REQUIRE(result.evaluated == 2U);
    REQUIRE(result.matched == 2U);
    REQUIRE(result.attempts == 5U);
    REQUIRE(result.retried == 3U);
    REQUIRE(result.delivered == 1U);
    REQUIRE(result.rejected == 4U);
    REQUIRE(result.exhausted == 1U);
    REQUIRE(!result.Complete());

    REQUIRE(transient_attempts == 3U);
    REQUIRE(permanent_attempts == 2U);

    const auto& last = bus.LastDeliveryResult();

    REQUIRE(last.attempts == 5U);
    REQUIRE(last.retried == 3U);
    REQUIRE(last.exhausted == 1U);

    return 0;
}
