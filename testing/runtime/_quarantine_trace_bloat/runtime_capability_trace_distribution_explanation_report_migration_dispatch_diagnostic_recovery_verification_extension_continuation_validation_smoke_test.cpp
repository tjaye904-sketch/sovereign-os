#include "runtime_service_capability_trace_report_migration_dispatch_diagnostic_recovery_verification_extension_continuation_validation.hpp"
#include "test_require.hpp"

int main()
{
    using namespace sovereign::runtime;

    REQUIRE(
        RuntimeServiceCapabilityTraceReportMigrationDispatchDiagnosticRecoveryVerificationExtensionContinuationValidation::validate()
    );

    REQUIRE(
        RuntimeServiceCapabilityTraceReportMigrationDispatchDiagnosticRecoveryVerificationExtensionContinuationValidation::name()
        ==
        "runtime_service_capability_trace_report_migration_dispatch_diagnostic_recovery_verification_extension_continuation_validation"
    );

    return 0;
}
