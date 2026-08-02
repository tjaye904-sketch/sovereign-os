#include "core/runtime/runtime_service_capability_trace_report_migration_dispatch_diagnostic_recovery_verification_integration_extension_continuation_validation.hpp"

#include "test_require.hpp"

int main()
{
    using Runtime =
        sovereign::runtime::CapabilityTraceMigrationDispatchDiagnosticRecoveryVerificationIntegrationExtensionContinuationValidation;

    REQUIRE(Runtime::enabled());
    REQUIRE(Runtime::name() != nullptr);

    return 0;
}
