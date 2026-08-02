#include "runtime_service_capability_trace_report_migration_dispatch_diagnostic_recovery_verification_extension.hpp"
#include "test_require.hpp"

using namespace sovereign::runtime;

int main()
{
    auto result =
        CapabilityTraceReportMigrationDispatchDiagnosticRecoveryVerificationExtension::verify();

    REQUIRE(
        result.code ==
        MigrationDispatchDiagnosticRecoveryVerificationExtensionCode::extension_verified);

    REQUIRE(!result.message.empty());

    return 0;
}
