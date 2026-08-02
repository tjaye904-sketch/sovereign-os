#include "runtime_service_capability_trace_report_migration_dispatch_diagnostic_recovery_verification_extension_continuation_validation_integration.hpp"
#include "test_require.hpp"

int main()
{
    using namespace sovereign::runtime;

    REQUIRE(
        RuntimeServiceCapabilityTraceReportMigrationDispatchDiagnosticRecoveryVerificationExtensionContinuationValidationIntegration::integrate()
    );

    REQUIRE(
        RuntimeServiceCapabilityTraceReportMigrationDispatchDiagnosticRecoveryVerificationExtensionContinuationValidationIntegration::name()
        ==
        "runtime_service_capability_trace_report_migration_dispatch_diagnostic_recovery_verification_extension_continuation_validation_integration"
    );

    return 0;
}
