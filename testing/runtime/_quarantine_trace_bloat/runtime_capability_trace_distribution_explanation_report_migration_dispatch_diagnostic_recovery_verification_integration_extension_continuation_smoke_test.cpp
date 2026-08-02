#include "core/runtime/runtime_service_capability_trace_report_migration_dispatch_diagnostic_recovery_verification_integration_extension_continuation.hpp"

int main() {
    return sovereign::runtime::
        CapabilityTraceMigrationDispatchDiagnosticRecoveryVerificationIntegrationExtensionContinuationRuntime::
            enabled()
        ? 0
        : 1;
}
