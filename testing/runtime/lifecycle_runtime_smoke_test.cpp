#include "lifecycle_manager.hpp"

#include "test_require.hpp"

int main()
{
    sovereign::LifecycleManager lifecycle;

    REQUIRE(
        lifecycle.State() ==
        sovereign::RuntimeState::Offline);

    REQUIRE(!lifecycle.Transition(
        sovereign::RuntimeState::Running));

    auto rejected =
        lifecycle.LastTransition();

    REQUIRE(!rejected.accepted);
    REQUIRE(
        rejected.previous ==
        sovereign::RuntimeState::Offline);
    REQUIRE(
        rejected.requested ==
        sovereign::RuntimeState::Running);
    REQUIRE(
        rejected.current ==
        sovereign::RuntimeState::Offline);

    REQUIRE(lifecycle.Transition(
        sovereign::RuntimeState::Initializing));

    REQUIRE(lifecycle.Transition(
        sovereign::RuntimeState::Starting));

    REQUIRE(lifecycle.Transition(
        sovereign::RuntimeState::Running));

    REQUIRE(lifecycle.Transition(
        sovereign::RuntimeState::Pausing));

    REQUIRE(lifecycle.Transition(
        sovereign::RuntimeState::Paused));

    REQUIRE(lifecycle.Transition(
        sovereign::RuntimeState::Resuming));

    REQUIRE(lifecycle.Transition(
        sovereign::RuntimeState::Running));

    REQUIRE(lifecycle.Transition(
        sovereign::RuntimeState::Stopping));

    REQUIRE(lifecycle.Transition(
        sovereign::RuntimeState::Stopped));

    REQUIRE(
        lifecycle.State() ==
        sovereign::RuntimeState::Stopped);

    REQUIRE(!lifecycle.Transition(
        sovereign::RuntimeState::Initializing));

    return 0;
}
