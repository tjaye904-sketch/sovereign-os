#include "event.hpp"

#include "test_require.hpp"

int main()
{
    sovereign::Event event;

    REQUIRE(
        event.metadata.category ==
        sovereign::EventCategory::Runtime);

    REQUIRE(
        event.metadata.priority ==
        sovereign::EventPriority::Normal);

    REQUIRE(
        event.metadata.security ==
        sovereign::EventSecurity::Internal);

    event.metadata.category =
        sovereign::EventCategory::Security;

    event.metadata.priority =
        sovereign::EventPriority::Critical;

    event.metadata.security =
        sovereign::EventSecurity::Restricted;

    REQUIRE(
        event.metadata.category ==
        sovereign::EventCategory::Security);

    REQUIRE(
        event.metadata.priority ==
        sovereign::EventPriority::Critical);

    REQUIRE(
        event.metadata.security ==
        sovereign::EventSecurity::Restricted);

    return 0;
}
