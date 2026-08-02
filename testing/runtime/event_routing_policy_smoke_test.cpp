#include "event_bus.hpp"

#include <cstddef>
#include <optional>
#include "test_require.hpp"

int main()
{
    sovereign::EventBus bus;

    std::size_t security_deliveries = 0U;
    std::size_t high_priority_deliveries = 0U;
    std::size_t confidential_deliveries = 0U;

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "",
                .category =
                    sovereign::EventCategory::Security,
                .minimum_priority = std::nullopt,
                .security_clearance =
                    sovereign::EventSecurity::Restricted,
                .retry_policy = {},
                .handler =
                    [&security_deliveries](
                        const sovereign::Event&)
                    {
                        ++security_deliveries;
                    }}));

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "",
                .category = std::nullopt,
                .minimum_priority =
                    sovereign::EventPriority::High,
                .security_clearance =
                    sovereign::EventSecurity::Internal,
                .retry_policy = {},
                .handler =
                    [&high_priority_deliveries](
                        const sovereign::Event&)
                    {
                        ++high_priority_deliveries;
                    }}));

    REQUIRE(
        bus.Subscribe(
            sovereign::EventSubscription{
                .topic = "security.audit",
                .category =
                    sovereign::EventCategory::Security,
                .minimum_priority =
                    sovereign::EventPriority::Critical,
                .security_clearance =
                    sovereign::EventSecurity::Confidential,
                .retry_policy = {},
                .handler =
                    [&confidential_deliveries](
                        const sovereign::Event&)
                    {
                        ++confidential_deliveries;
                    }}));

    sovereign::Event normal_runtime;
    normal_runtime.topic = "runtime.tick";
    normal_runtime.metadata.category =
        sovereign::EventCategory::Runtime;
    normal_runtime.metadata.priority =
        sovereign::EventPriority::Normal;
    normal_runtime.metadata.security =
        sovereign::EventSecurity::Internal;

    sovereign::Event critical_security;
    critical_security.topic = "security.audit";
    critical_security.metadata.category =
        sovereign::EventCategory::Security;
    critical_security.metadata.priority =
        sovereign::EventPriority::Critical;
    critical_security.metadata.security =
        sovereign::EventSecurity::Restricted;

    sovereign::Event confidential_security;
    confidential_security.topic = "security.audit";
    confidential_security.metadata.category =
        sovereign::EventCategory::Security;
    confidential_security.metadata.priority =
        sovereign::EventPriority::Emergency;
    confidential_security.metadata.security =
        sovereign::EventSecurity::Confidential;

    REQUIRE(bus.Publish(normal_runtime));
    REQUIRE(bus.Publish(critical_security));
    REQUIRE(bus.Publish(confidential_security));
    REQUIRE(bus.Process());

    REQUIRE(security_deliveries == 1U);
    REQUIRE(high_priority_deliveries == 0U);
    REQUIRE(confidential_deliveries == 2U);

    return 0;
}
