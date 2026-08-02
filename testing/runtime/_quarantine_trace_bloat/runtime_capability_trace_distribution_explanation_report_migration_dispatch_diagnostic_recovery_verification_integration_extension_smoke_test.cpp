#include "runtime_service_capability_trace_report_migration_dispatch_diagnostic_recovery_verification_integration_extension.hpp"

int main() {
    using namespace sovereign::runtime;

    if (!CapabilityTraceMigrationDispatchDiagnosticRecoveryVerificationIntegrationExtension::enabled()) {
        return 1;
    }

    return 0;
}
