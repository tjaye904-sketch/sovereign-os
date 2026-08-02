#pragma once

#include <string>

namespace sovereign::runtime {

class CapabilityTraceReportMigrationDispatchDiagnosticRecoveryVerification {
public:
    enum class Code {
        ok,
        recovery_state_verified,
        recovery_state_mismatch,
        verification_failed
    };

    struct Result {
        Code code;
        bool verified;
        std::string message;
    };

    Result verify(
        bool recovery_completed,
        bool migration_dispatch_valid,
        bool diagnostic_state_valid
    ) const {
        if (!recovery_completed) {
            return {
                Code::verification_failed,
                false,
                "recovery not completed"
            };
        }

        if (!migration_dispatch_valid ||
            !diagnostic_state_valid) {
            return {
                Code::recovery_state_mismatch,
                false,
                "recovered state failed verification"
            };
        }

        return {
            Code::recovery_state_verified,
            true,
            "recovered state verified"
        };
    }
};

} // namespace sovereign::runtime
