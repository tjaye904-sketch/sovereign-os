#include "health_monitor.hpp"

#include <string>
#include "test_require.hpp"

int main()
{
    sovereign::HealthMonitor monitor;

    REQUIRE(monitor.RecordCount() == 0U);
    REQUIRE(
        monitor.OverallState() ==
        sovereign::HealthState::Unknown);

    sovereign::HealthRecord invalid;
    invalid.state = sovereign::HealthState::Healthy;

    REQUIRE(!monitor.Report(invalid));

    sovereign::HealthRecord runtime;
    runtime.component = "Runtime";
    runtime.state = sovereign::HealthState::Healthy;
    runtime.message = "Runtime operational.";

    sovereign::HealthRecord scheduler;
    scheduler.component = "Scheduler";
    scheduler.state = sovereign::HealthState::Degraded;
    scheduler.message = "Frame budget pressure.";

    sovereign::HealthRecord memory;
    memory.component = "Memory";
    memory.state = sovereign::HealthState::Critical;
    memory.message = "Memory capacity exhausted.";

    REQUIRE(monitor.Report(runtime));
    REQUIRE(monitor.Report(scheduler));
    REQUIRE(monitor.Report(memory));

    REQUIRE(monitor.RecordCount() == 3U);

    REQUIRE(
        monitor.OverallState() ==
        sovereign::HealthState::Critical);

    const auto latest_scheduler =
        monitor.Latest("Scheduler");

    REQUIRE(latest_scheduler.has_value());
    REQUIRE(
        latest_scheduler->state ==
        sovereign::HealthState::Degraded);
    REQUIRE(
        latest_scheduler->message ==
        "Frame budget pressure.");

    REQUIRE(!monitor.Latest("Missing").has_value());

    sovereign::HealthRecord scheduler_recovery;
    scheduler_recovery.component = "Scheduler";
    scheduler_recovery.state = sovereign::HealthState::Healthy;
    scheduler_recovery.message = "Scheduler recovered.";

    REQUIRE(monitor.Report(scheduler_recovery));

    const auto recovered_scheduler =
        monitor.Latest("Scheduler");

    REQUIRE(recovered_scheduler.has_value());
    REQUIRE(
        recovered_scheduler->state ==
        sovereign::HealthState::Healthy);
    REQUIRE(
        recovered_scheduler->message ==
        "Scheduler recovered.");

    REQUIRE(
        monitor.OverallState() ==
        sovereign::HealthState::Critical);

    monitor.Clear();

    REQUIRE(monitor.RecordCount() == 0U);
    REQUIRE(
        monitor.OverallState() ==
        sovereign::HealthState::Unknown);

    return 0;
}
