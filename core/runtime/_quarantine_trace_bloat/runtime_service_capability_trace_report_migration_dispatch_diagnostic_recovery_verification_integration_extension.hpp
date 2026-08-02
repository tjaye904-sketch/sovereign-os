#pragma once

namespace sovereign::runtime {

struct CapabilityTraceMigrationDispatchDiagnosticRecoveryVerificationIntegrationExtension {
    static constexpr const char* name() {
        return "capability_trace_migration_dispatch_diagnostic_recovery_verification_integration_extension";
    }

    static constexpr bool enabled() {
        return true;
    }
};

}
