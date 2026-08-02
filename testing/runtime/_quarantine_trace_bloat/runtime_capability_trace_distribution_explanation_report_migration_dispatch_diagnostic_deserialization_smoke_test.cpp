#include <string>

#include "runtime_service_capability_trace_report_migration_dispatch_diagnostic_deserialization.hpp"
#include "test_require.hpp"

using namespace sovereign::runtime;

int main() {

    {
        const std::string payload =
            R"({"schema_version":"v0.1.0","migration_id":"migration-152","status":"complete"})";

        auto result =
            MigrationDispatchDiagnosticDeserializer::deserialize(payload);

        REQUIRE(result.success);
        REQUIRE(result.code == "ok");
        REQUIRE(result.schema_version == "v0.1.0");
        REQUIRE(result.migration_id == "migration-152");
        REQUIRE(result.status == "complete");
    }

    {
        auto result =
            MigrationDispatchDiagnosticDeserializer::deserialize("");

        REQUIRE(!result.success);
        REQUIRE(result.code == "malformed_payload");
    }

    {
        const std::string payload =
            R"({"migration_id":"migration-152","status":"complete"})";

        auto result =
            MigrationDispatchDiagnosticDeserializer::deserialize(payload);

        REQUIRE(!result.success);
        REQUIRE(result.code == "missing_field");
    }

    {
        const std::string payload =
            R"({"schema_version":"bad","migration_id":"migration-152","status":"complete"})";

        auto result =
            MigrationDispatchDiagnosticDeserializer::deserialize(payload);

        REQUIRE(!result.success);
        REQUIRE(result.code == "schema_version_malformed");
    }

    return 0;
}
