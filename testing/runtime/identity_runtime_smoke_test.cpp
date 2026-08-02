#include "identity_types.hpp"
#include "uuid.hpp"

#include <string>
#include "test_require.hpp"

int main()
{
    const auto nil =
        sovereign::Uuid::Nil();

    REQUIRE(nil.IsNil());
    REQUIRE(
        nil.ToString() ==
        "00000000-0000-0000-0000-000000000000");

    const auto generated =
        sovereign::Uuid::Generate();

    REQUIRE(!generated.IsNil());

    const std::string serialized =
        generated.ToString();

    REQUIRE(serialized.size() == 36U);
    REQUIRE(serialized[8] == '-');
    REQUIRE(serialized[13] == '-');
    REQUIRE(serialized[18] == '-');
    REQUIRE(serialized[23] == '-');

    const auto parsed =
        sovereign::Uuid::Parse(serialized);

    REQUIRE(parsed.has_value());
    REQUIRE(*parsed == generated);

    REQUIRE(!sovereign::Uuid::Parse("").has_value());

    REQUIRE(!sovereign::Uuid::Parse(
        "invalid-uuid-value").has_value());

    REQUIRE(!sovereign::Uuid::Parse(
        "00000000-0000-0000-0000-00000000000z")
        .has_value());

    const auto runtime_id =
        sovereign::RuntimeId::Generate();

    const auto service_id =
        sovereign::ServiceId::Generate();

    REQUIRE(!runtime_id.IsNil());
    REQUIRE(!service_id.IsNil());

    const auto parsed_runtime_uuid =
        sovereign::Uuid::Parse(
            runtime_id.ToString());

    REQUIRE(parsed_runtime_uuid.has_value());
    REQUIRE(
        *parsed_runtime_uuid ==
        runtime_id.Value());

    const auto nil_event_id =
        sovereign::EventId::Nil();

    REQUIRE(nil_event_id.IsNil());

    return 0;
}
