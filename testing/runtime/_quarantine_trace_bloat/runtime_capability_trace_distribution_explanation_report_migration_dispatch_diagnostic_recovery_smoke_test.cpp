#include "runtime_service_capability_trace_report_migration_dispatch_diagnostic_recovery.hpp"
#include "test_require.hpp"

using namespace sovereign::runtime;

int main()
{
    CapabilityTraceMigrationDispatchDiagnosticRecovery recovery;

    auto recoverable =
        recovery.recover("migration_id_missing");

    REQUIRE(recoverable.recovered);
    REQUIRE(
        recoverable.state ==
        CapabilityTraceMigrationRecoveryState::recoverable);

    REQUIRE(
        recoverable.code ==
        "recovery_completed");


    auto terminal =
        recovery.recover("schema_version_missing");

    REQUIRE(!terminal.recovered);
    REQUIRE(
        terminal.state ==
        CapabilityTraceMigrationRecoveryState::terminal);

    REQUIRE(
        terminal.code ==
        "recovery_terminal_schema_missing");

    return 0;
}
