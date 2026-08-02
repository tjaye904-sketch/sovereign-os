#pragma once

#include <optional>
#include <sstream>
#include <string>

namespace sovereign::runtime {

struct MigrationDispatchDiagnosticDeserializationResult {
    bool success{false};
    std::string code;
    std::string message;
    std::string schema_version;
    std::string migration_id;
    std::string status;
    std::string serialized_source;

    static MigrationDispatchDiagnosticDeserializationResult failure(
        std::string code_value,
        std::string message_value) {
        MigrationDispatchDiagnosticDeserializationResult result;
        result.code = std::move(code_value);
        result.message = std::move(message_value);
        return result;
    }
};

class MigrationDispatchDiagnosticDeserializer {
public:
    static MigrationDispatchDiagnosticDeserializationResult deserialize(
        const std::string& payload) {

        if (payload.empty()) {
            return MigrationDispatchDiagnosticDeserializationResult::failure(
                "malformed_payload",
                "payload is empty");
        }

        auto schema = extract(payload, "schema_version");
        auto migration = extract(payload, "migration_id");
        auto status = extract(payload, "status");

        if (!schema.has_value()) {
            return MigrationDispatchDiagnosticDeserializationResult::failure(
                "missing_field",
                "schema_version missing");
        }

        if (!migration.has_value()) {
            return MigrationDispatchDiagnosticDeserializationResult::failure(
                "missing_field",
                "migration_id missing");
        }

        if (!status.has_value()) {
            return MigrationDispatchDiagnosticDeserializationResult::failure(
                "missing_field",
                "status missing");
        }

        if (*schema != "v0.1.0") {
            return MigrationDispatchDiagnosticDeserializationResult::failure(
                "schema_version_malformed",
                "unsupported schema version");
        }

        MigrationDispatchDiagnosticDeserializationResult result;
        result.success = true;
        result.code = "ok";
        result.message = "deserialization successful";
        result.schema_version = *schema;
        result.migration_id = *migration;
        result.status = *status;
        result.serialized_source = payload;

        return result;
    }

private:
    static std::optional<std::string> extract(
        const std::string& payload,
        const std::string& key) {

        const std::string token = "\"" + key + "\"";

        auto key_position = payload.find(token);

        if (key_position == std::string::npos) {
            return std::nullopt;
        }

        auto colon = payload.find(':', key_position);

        if (colon == std::string::npos) {
            return std::nullopt;
        }

        auto first_quote = payload.find('"', colon + 1);

        if (first_quote == std::string::npos) {
            return std::nullopt;
        }

        auto second_quote = payload.find('"', first_quote + 1);

        if (second_quote == std::string::npos) {
            return std::nullopt;
        }

        return payload.substr(
            first_quote + 1,
            second_quote - first_quote - 1);
    }
};

}
