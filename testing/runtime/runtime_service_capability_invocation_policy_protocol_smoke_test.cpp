#include "runtime_service_capability_invocation_policy_protocol.hpp"

#include <string>
#include "test_require.hpp"

namespace
{

using Command =
    sovereign::
        RuntimeServiceCapabilityInvocationPolicyCommand;

using Protocol =
    sovereign::
        RuntimeServiceCapabilityInvocationPolicyProtocol;

using ProtocolRequest =
    sovereign::
        RuntimeServiceCapabilityInvocationPolicyProtocolRequest;

using ProtocolResult =
    sovereign::
        RuntimeServiceCapabilityInvocationPolicyProtocolResult;

using ProtocolStatus =
    sovereign::
        RuntimeServiceCapabilityInvocationPolicyProtocolStatus;

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
        decoded->operation ==
        request.operation);

    REQUIRE(
        decoded->provider_service_name ==
        request.provider_service_name);
}

} // namespace

int main()
{
    /*
     * All supported control commands round-trip.
     */
    VerifyRequestRoundTrip(
        ProtocolRequest{
            .command = Command::DenyCapability,
            .capability = "analysis.generate"});

    VerifyRequestRoundTrip(
        ProtocolRequest{
            .command = Command::DenyOperation,
            .capability = "analysis.generate",
            .operation = "summarize"});

    VerifyRequestRoundTrip(
        ProtocolRequest{
            .command = Command::DenyProvider,
            .capability = "analysis.generate",
            .provider_service_name =
                "RestrictedAnalysisService"});

    VerifyRequestRoundTrip(
        ProtocolRequest{
            .command = Command::Query});

    /*
     * Length-prefix framing preserves protocol
     * delimiters inside every policy key.
     */
    VerifyRequestRoundTrip(
        ProtocolRequest{
            .command = Command::DenyOperation,
            .capability =
                "analysis:generate|primary",
            .operation =
                "summarize|private:document"});

    VerifyRequestRoundTrip(
        ProtocolRequest{
            .command = Command::DenyProvider,
            .capability =
                "analysis:generate|primary",
            .provider_service_name =
                "Restricted|Analysis:East"});

    /*
     * Control result round-trip.
     */
    const ProtocolResult source_result{
        .status = ProtocolStatus::Accepted,
        .denied_capability_count = 7U,
        .denied_operation_count = 11U,
        .denied_provider_count = 13U,
        .diagnostic = "policy:control|snapshot"};

    const std::string encoded_result =
        Protocol::EncodeResult(source_result);

    const auto decoded_result =
        Protocol::DecodeResult(encoded_result);

    REQUIRE(decoded_result.has_value());
    REQUIRE(decoded_result->Accepted());

    REQUIRE(
        decoded_result->denied_capability_count ==
        7U);

    REQUIRE(
        decoded_result->denied_operation_count ==
        11U);

    REQUIRE(
        decoded_result->denied_provider_count ==
        13U);

    REQUIRE(
        decoded_result->diagnostic ==
        "policy:control|snapshot");

    /*
     * Existing decision serialization remains
     * compatible after adding control messages.
     */
    const sovereign::
        RuntimeServiceCapabilityInvocationPolicyDecision
            source_decision{
                .status =
                    sovereign::
                        RuntimeServiceCapabilityInvocationPolicyStatus::
                            ProviderDenied,
                .capability =
                    "analysis:generate",
                .operation =
                    "summarize|private",
                .provider_service_name =
                    "Restricted:Analysis|East",
                .diagnostic =
                    "provider:denied|policy"};

    const std::string encoded_decision =
        Protocol::EncodeDecision(
            source_decision);

    const auto decoded_decision =
        Protocol::DecodeDecision(
            encoded_decision);

    REQUIRE(decoded_decision.has_value());

    REQUIRE(
        decoded_decision->status ==
        source_decision.status);

    REQUIRE(
        decoded_decision->capability ==
        source_decision.capability);

    REQUIRE(
        decoded_decision->operation ==
        source_decision.operation);

    REQUIRE(
        decoded_decision
            ->provider_service_name ==
        source_decision
            .provider_service_name);

    REQUIRE(
        decoded_decision->diagnostic ==
        source_decision.diagnostic);

    /*
     * Unsupported versions are rejected.
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
        "0:";

    REQUIRE(
        !Protocol::DecodeRequest(
             unknown_command)
             .has_value());

    /*
     * Invalid command-field combinations are
     * rejected.
     */
    const std::string invalid_capability =
        Protocol::EncodeRequest(
            ProtocolRequest{
                .command =
                    Command::DenyCapability,
                .capability =
                    "analysis.generate",
                .operation =
                    "must-be-empty"});

    REQUIRE(
        !Protocol::DecodeRequest(
             invalid_capability)
             .has_value());

    const std::string invalid_operation =
        Protocol::EncodeRequest(
            ProtocolRequest{
                .command =
                    Command::DenyOperation,
                .capability =
                    "analysis.generate"});

    REQUIRE(
        !Protocol::DecodeRequest(
             invalid_operation)
             .has_value());

    const std::string invalid_provider =
        Protocol::EncodeRequest(
            ProtocolRequest{
                .command =
                    Command::DenyProvider,
                .capability =
                    "analysis.generate"});

    REQUIRE(
        !Protocol::DecodeRequest(
             invalid_provider)
             .has_value());

    const std::string invalid_query =
        Protocol::EncodeRequest(
            ProtocolRequest{
                .command = Command::Query,
                .capability =
                    "must-be-empty"});

    REQUIRE(
        !Protocol::DecodeRequest(
             invalid_query)
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

    REQUIRE(
        !Protocol::DecodeDecision(
             encoded_decision.substr(
                 0U,
                 encoded_decision.size() - 1U))
             .has_value());

    /*
     * Out-of-range result status is rejected.
     */
    const std::string invalid_status =
        "1:1"
        "3:255"
        "1:0"
        "1:0"
        "1:0"
        "0:";

    REQUIRE(
        !Protocol::DecodeResult(
             invalid_status)
             .has_value());

    return 0;
}
