#pragma once

#include <string>

namespace sovereign::runtime
{

struct CapabilityTraceMigrationDiagnosticValidationResult
{
    bool valid{false};
    std::string code;
    std::string message;
};

class CapabilityTraceMigrationDispatchDiagnosticValidator
{
public:

    CapabilityTraceMigrationDiagnosticValidationResult validate(
        const std::string& schema_version,
        const std::string& migration_id,
        const std::string& dispatch_code) const
    {
        CapabilityTraceMigrationDiagnosticValidationResult result;

        if (schema_version.empty())
        {
            result.code = "schema_version_missing";
            result.message = "migration diagnostic schema version missing";
            return result;
        }

        if (migration_id.empty())
        {
            result.code = "migration_id_missing";
            result.message = "migration diagnostic migration id missing";
            return result;
        }

        if (dispatch_code.empty())
        {
            result.code = "dispatch_code_missing";
            result.message = "migration diagnostic dispatch code missing";
            return result;
        }

        result.valid = true;
        result.code = "ok";
        result.message = "migration diagnostic validation succeeded";

        return result;
    }
};

}
