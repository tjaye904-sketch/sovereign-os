#include "runtime_service_capability_trace_report_migration_dispatch_diagnostic_validation.hpp"
#include "test_require.hpp"

using namespace sovereign::runtime;

int main()
{
    CapabilityTraceMigrationDispatchDiagnosticValidator validator;

    auto valid =
        validator.validate(
            "1.0",
            "migration_152",
            "dispatch_ok");

    REQUIRE(valid.valid);
    REQUIRE(valid.code == "ok");

    auto invalid =
        validator.validate(
            "",
            "migration_152",
            "dispatch_ok");

    REQUIRE(!invalid.valid);
    REQUIRE(invalid.code == "schema_version_missing");

    return 0;
}
