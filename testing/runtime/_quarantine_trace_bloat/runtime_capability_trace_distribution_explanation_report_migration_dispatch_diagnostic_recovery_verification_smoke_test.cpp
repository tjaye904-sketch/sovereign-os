#include "runtime_service_capability_trace_report_migration_dispatch_diagnostic_recovery_verification.hpp"
#include "test_require.hpp"

using namespace sovereign::runtime;

int main() {
    CapabilityTraceReportMigrationDispatchDiagnosticRecoveryVerification verifier;

    auto valid =
        verifier.verify(
            true,
            true,
            true
        );

    REQUIRE(
        valid.code ==
        CapabilityTraceReportMigrationDispatchDiagnosticRecoveryVerification::Code::recovery_state_verified
    );

    REQUIRE(valid.verified);

    auto invalid =
        verifier.verify(
            true,
            false,
            true
        );

    REQUIRE(
        invalid.code ==
        CapabilityTraceReportMigrationDispatchDiagnosticRecoveryVerification::Code::recovery_state_mismatch
    );

    REQUIRE(!invalid.verified);

    return 0;
}
