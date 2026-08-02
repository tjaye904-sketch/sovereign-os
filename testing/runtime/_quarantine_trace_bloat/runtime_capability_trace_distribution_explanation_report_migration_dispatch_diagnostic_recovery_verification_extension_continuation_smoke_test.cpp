#include "runtime_service_capability_trace_report_migration_dispatch_diagnostic_recovery_verification_extension_continuation.hpp"

#include "test_require.hpp"

int main()
{
    using Runtime =
        sovereign::runtime::
        CapabilityTraceReportMigrationDispatchDiagnosticRecoveryVerificationExtensionContinuation;

    REQUIRE(Runtime::validate());
    REQUIRE(Runtime::recover());

    return 0;
}
