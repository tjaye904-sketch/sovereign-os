#include "runtime_service_capability_provider_selection_policy.hpp"
#include "runtime_service_capability_provider_selection_policy_protocol.hpp"

#include <cstdint>
#include <string>
#include "test_require.hpp"

namespace
{

using Command =
    sovereign::
        RuntimeServiceCapabilityProviderSelectionPolicyCommand;

using Protocol =
    sovereign::
        RuntimeServiceCapabilityProviderSelectionPolicyProtocol;

using ProtocolRequest =
    sovereign::
        RuntimeServiceCapabilityProviderSelectionPolicyProtocolRequest;

using ProtocolResult =
    sovereign::
        RuntimeServiceCapabilityProviderSelectionPolicyProtocolResult;

using ProtocolStatus =
    sovereign::
        RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus;

void VerifyRequestRoundTrip(
    const ProtocolRequest& request)
{
    const std::string encoded =
        Protocol::EncodeRequest(request);

    const auto decoded =
        Protocol::DecodeRequest(encoded);

    REQUIRE(decoded.has_value());

    REQUIRE(
        decoded->command ==
        request.command);

    REQUIRE(
        decoded->capability ==
        request.capability);

    REQUIRE(
        decoded->service_name ==
        request.service_name);

    REQUIRE(decoded->rank == request.rank);

    REQUIRE(
        decoded
            ->preferred_provider_fallback_enabled ==
        request
            .preferred_provider_fallback_enabled);
}

} // namespace

int main()
{
    /*
     * Set-rank request round-trip.
     */
    VerifyRequestRoundTrip(
        ProtocolRequest{
            .command = Command::SetRank,
            .capability = "storage.read",
            .service_name = "ReplicaStorage",
            .rank = 10U});

    /*
     * Remove-rank request round-trip.
     */
    VerifyRequestRoundTrip(
        ProtocolRequest{
            .command = Command::RemoveRank,
            .capability = "storage.read",
            .service_name = "ReplicaStorage"});

    /*
     * Preferred-provider fallback request
     * round-trip.
     */
    VerifyRequestRoundTrip(
        ProtocolRequest{
            .command = Command::SetFallback,
            .preferred_provider_fallback_enabled =
                true});

    /*
     * Query request round-trip.
     */
    VerifyRequestRoundTrip(
        ProtocolRequest{
            .command = Command::Query});

    /*
     * Length-prefix framing preserves delimiters.
     */
    VerifyRequestRoundTrip(
        ProtocolRequest{
            .command = Command::SetRank,
            .capability = "storage:read|primary",
            .service_name =
                "Replica|Storage:East",
            .rank = 17U});

    sovereign::
        RuntimeServiceCapabilityProviderSelectionPolicy
            policy;

    REQUIRE(
        policy.SetProviderRank(
            "storage:read",
            "Replica|Storage",
            10U));

    REQUIRE(
        policy.SetProviderRank(
            "storage.write",
            "Primary:Storage",
            5U));

    policy.SetPreferredProviderFallbackEnabled(
        true);

    ProtocolResult source_result{
        .status = ProtocolStatus::Accepted,
        .provider_ranks =
            policy.ProviderRanks(),
        .selection_count = 101U,
        .ranked_selection_count = 61U,
        .preferred_selection_count = 23U,
        .preferred_fallback_count = 7U,
        .preferred_provider_fallback_enabled =
            policy
                .PreferredProviderFallbackEnabled(),
        .diagnostic = "diag:abc|xyz"};

    const std::string encoded_result =
        Protocol::EncodeResult(source_result);

    const auto decoded_result =
        Protocol::DecodeResult(encoded_result);

    REQUIRE(decoded_result.has_value());
    REQUIRE(decoded_result->Accepted());

    REQUIRE(
        decoded_result->provider_ranks.size() ==
        2U);

    /*
     * Provider-rank ordering survives the protocol
     * round-trip.
     */
    REQUIRE(
        decoded_result
            ->provider_ranks[0U]
            .capability ==
        "storage:read");

    REQUIRE(
        decoded_result
            ->provider_ranks[0U]
            .service_name ==
        "Replica|Storage");

    REQUIRE(
        decoded_result
            ->provider_ranks[0U]
            .rank ==
        10U);

    REQUIRE(
        decoded_result
            ->provider_ranks[1U]
            .capability ==
        "storage.write");

    REQUIRE(
        decoded_result
            ->provider_ranks[1U]
            .service_name ==
        "Primary:Storage");

    REQUIRE(
        decoded_result
            ->provider_ranks[1U]
            .rank ==
        5U);

    /*
     * Policy counters and fallback state survive the
     * result round-trip.
     */
    REQUIRE(
        decoded_result->selection_count ==
        101U);

    REQUIRE(
        decoded_result
            ->ranked_selection_count ==
        61U);

    REQUIRE(
        decoded_result
            ->preferred_selection_count ==
        23U);

    REQUIRE(
        decoded_result
            ->preferred_fallback_count ==
        7U);

    REQUIRE(
        decoded_result
            ->preferred_provider_fallback_enabled);

    REQUIRE(
        decoded_result->diagnostic ==
        "diag:abc|xyz");

    /*
     * Unsupported versions are rejected.
     *
     * Encoded query begins with:
     *     1:1
     *
     * Replace version value 1 with 2.
     */
    std::string invalid_version =
        Protocol::EncodeRequest(
            ProtocolRequest{
                .command = Command::Query});

    REQUIRE(invalid_version.size() >= 3U);
    invalid_version[2U] = '2';

    REQUIRE(
        !Protocol::DecodeRequest(
             invalid_version)
             .has_value());

    /*
     * Unknown commands are rejected.
     */
    const std::string unknown_command =
        "1:1"
        "7:UNKNOWN"
        "0:"
        "0:"
        "1:0"
        "1:0";

    REQUIRE(
        !Protocol::DecodeRequest(
             unknown_command)
             .has_value());

    /*
     * Malformed and truncated payloads are rejected.
     */
    REQUIRE(
        !Protocol::DecodeRequest(
             "1:1")
             .has_value());

    REQUIRE(
        !Protocol::DecodeResult(
             encoded_result.substr(
                 0U,
                 encoded_result.size() - 1U))
             .has_value());

    /*
     * Invalid command-field combinations are
     * rejected.
     */
    const std::string invalid_query =
        Protocol::EncodeRequest(
            ProtocolRequest{
                .command = Command::Query,
                .capability = "not-empty"});

    REQUIRE(
        !Protocol::DecodeRequest(
             invalid_query)
             .has_value());

    return 0;
}
