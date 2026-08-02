#pragma once

#include <string>

namespace sovereign::runtime
{

enum class CapabilityTraceMigrationRecoveryState
{
    recoverable,
    terminal
};

struct CapabilityTraceMigrationDiagnosticRecoveryResult
{
    bool recovered{false};
    CapabilityTraceMigrationRecoveryState state{
        CapabilityTraceMigrationRecoveryState::terminal
    };
    std::string code;
    std::string message;
};

class CapabilityTraceMigrationDispatchDiagnosticRecovery
{
public:

    CapabilityTraceMigrationDiagnosticRecoveryResult recover(
        const std::string& validation_code) const
    {
        CapabilityTraceMigrationDiagnosticRecoveryResult result;

        if (validation_code == "schema_version_missing")
        {
            result.recovered = false;
            result.state =
                CapabilityTraceMigrationRecoveryState::terminal;
            result.code = "recovery_terminal_schema_missing";
            result.message =
                "migration diagnostic recovery failed: schema missing";
            return result;
        }

        if (validation_code == "migration_id_missing" ||
            validation_code == "dispatch_code_missing")
        {
            result.recovered = true;
            result.state =
                CapabilityTraceMigrationRecoveryState::recoverable;
            result.code = "recovery_completed";
            result.message =
                "migration diagnostic recovered from incomplete state";
            return result;
        }

        result.recovered = true;
        result.state =
            CapabilityTraceMigrationRecoveryState::recoverable;
        result.code = "recovery_not_required";
        result.message =
            "migration diagnostic state accepted";
        
        return result;
    }
};

}
