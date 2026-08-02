#include "runtime_service_capability_invocation_policy_protocol.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

namespace sovereign
{
namespace
{

constexpr std::string_view ProtocolVersion{"1"};

void AppendField(
    std::string& encoded,
    const std::string_view value)
{
    encoded.append(std::to_string(value.size()));
    encoded.push_back(':');
    encoded.append(value);
}

bool ReadField(
    const std::string_view encoded,
    std::size_t& offset,
    std::string& value)
{
    if (offset >= encoded.size())
    {
        return false;
    }

    const std::size_t delimiter =
        encoded.find(':', offset);

    if (delimiter == std::string_view::npos ||
        delimiter == offset)
    {
        return false;
    }

    std::size_t length{0U};

    const char* begin =
        encoded.data() + offset;

    const char* end =
        encoded.data() + delimiter;

    const auto parsed =
        std::from_chars(begin, end, length);

    if (parsed.ec != std::errc{} ||
        parsed.ptr != end)
    {
        return false;
    }

    const std::size_t value_offset =
        delimiter + 1U;

    if (length > encoded.size() - value_offset)
    {
        return false;
    }

    value.assign(
        encoded.substr(value_offset, length));

    offset = value_offset + length;
    return true;
}

template<typename Integer>
bool ParseInteger(
    const std::string_view encoded,
    Integer& value)
{
    static_assert(
        std::is_integral_v<Integer>);

    if (encoded.empty())
    {
        return false;
    }

    const char* begin = encoded.data();
    const char* end = begin + encoded.size();

    const auto parsed =
        std::from_chars(begin, end, value);

    return
        parsed.ec == std::errc{} &&
        parsed.ptr == end;
}

std::string_view EncodeCommand(
    const
    RuntimeServiceCapabilityInvocationPolicyCommand
        command)
{
    switch (command)
    {
        case
            RuntimeServiceCapabilityInvocationPolicyCommand::
                DenyCapability:
            return "DENY_CAPABILITY";

        case
            RuntimeServiceCapabilityInvocationPolicyCommand::
                DenyOperation:
            return "DENY_OPERATION";

        case
            RuntimeServiceCapabilityInvocationPolicyCommand::
                DenyProvider:
            return "DENY_PROVIDER";

        case
            RuntimeServiceCapabilityInvocationPolicyCommand::
                Query:
            return "QUERY";
    }

    return "UNKNOWN";
}

bool DecodeCommand(
    const std::string_view encoded,
    RuntimeServiceCapabilityInvocationPolicyCommand&
        command)
{
    if (encoded == "DENY_CAPABILITY")
    {
        command =
            RuntimeServiceCapabilityInvocationPolicyCommand::
                DenyCapability;

        return true;
    }

    if (encoded == "DENY_OPERATION")
    {
        command =
            RuntimeServiceCapabilityInvocationPolicyCommand::
                DenyOperation;

        return true;
    }

    if (encoded == "DENY_PROVIDER")
    {
        command =
            RuntimeServiceCapabilityInvocationPolicyCommand::
                DenyProvider;

        return true;
    }

    if (encoded == "QUERY")
    {
        command =
            RuntimeServiceCapabilityInvocationPolicyCommand::
                Query;

        return true;
    }

    return false;
}

bool ValidRequest(
    const
    RuntimeServiceCapabilityInvocationPolicyProtocolRequest&
        request)
{
    using Command =
        RuntimeServiceCapabilityInvocationPolicyCommand;

    switch (request.command)
    {
        case Command::DenyCapability:
            return
                !request.capability.empty() &&
                request.operation.empty() &&
                request.provider_service_name.empty();

        case Command::DenyOperation:
            return
                !request.capability.empty() &&
                !request.operation.empty() &&
                request.provider_service_name.empty();

        case Command::DenyProvider:
            return
                !request.capability.empty() &&
                request.operation.empty() &&
                !request.provider_service_name.empty();

        case Command::Query:
            return
                request.capability.empty() &&
                request.operation.empty() &&
                request.provider_service_name.empty();
    }

    return false;
}

} // namespace

std::string
RuntimeServiceCapabilityInvocationPolicyProtocol::
EncodeDecision(
    const
    RuntimeServiceCapabilityInvocationPolicyDecision&
        decision)
{
    std::string encoded{};

    AppendField(encoded, ProtocolVersion);

    AppendField(
        encoded,
        std::to_string(
            static_cast<std::uint8_t>(
                decision.status)));

    AppendField(encoded, decision.capability);
    AppendField(encoded, decision.operation);

    AppendField(
        encoded,
        decision.provider_service_name);

    AppendField(encoded, decision.diagnostic);

    return encoded;
}

std::optional<
    RuntimeServiceCapabilityInvocationPolicyDecision>
RuntimeServiceCapabilityInvocationPolicyProtocol::
DecodeDecision(const std::string_view payload)
{
    std::size_t offset{0U};

    std::string version{};
    std::string status{};

    RuntimeServiceCapabilityInvocationPolicyDecision
        decision{};

    if (!ReadField(payload, offset, version) ||
        version != ProtocolVersion ||
        !ReadField(payload, offset, status) ||
        !ReadField(
            payload,
            offset,
            decision.capability) ||
        !ReadField(
            payload,
            offset,
            decision.operation) ||
        !ReadField(
            payload,
            offset,
            decision.provider_service_name) ||
        !ReadField(
            payload,
            offset,
            decision.diagnostic) ||
        offset != payload.size())
    {
        return std::nullopt;
    }

    std::uint32_t raw_status{0U};

    if (!ParseInteger(status, raw_status) ||
        raw_status >
            static_cast<std::uint32_t>(
                RuntimeServiceCapabilityInvocationPolicyStatus::
                    ProviderDenied))
    {
        return std::nullopt;
    }

    decision.status =
        static_cast<
            RuntimeServiceCapabilityInvocationPolicyStatus>(
                raw_status);

    return decision;
}

std::string
RuntimeServiceCapabilityInvocationPolicyProtocol::
EncodeRequest(
    const
    RuntimeServiceCapabilityInvocationPolicyProtocolRequest&
        request)
{
    std::string encoded{};

    AppendField(encoded, ProtocolVersion);

    AppendField(
        encoded,
        EncodeCommand(request.command));

    AppendField(
        encoded,
        request.capability);

    AppendField(
        encoded,
        request.operation);

    AppendField(
        encoded,
        request.provider_service_name);

    return encoded;
}

std::optional<
    RuntimeServiceCapabilityInvocationPolicyProtocolRequest>
RuntimeServiceCapabilityInvocationPolicyProtocol::
DecodeRequest(const std::string_view payload)
{
    std::size_t offset{0U};

    std::string version{};
    std::string command{};

    RuntimeServiceCapabilityInvocationPolicyProtocolRequest
        request{};

    if (!ReadField(payload, offset, version) ||
        version != ProtocolVersion ||
        !ReadField(payload, offset, command) ||
        !ReadField(
            payload,
            offset,
            request.capability) ||
        !ReadField(
            payload,
            offset,
            request.operation) ||
        !ReadField(
            payload,
            offset,
            request.provider_service_name) ||
        offset != payload.size() ||
        !DecodeCommand(
            command,
            request.command) ||
        !ValidRequest(request))
    {
        return std::nullopt;
    }

    return request;
}

std::string
RuntimeServiceCapabilityInvocationPolicyProtocol::
EncodeResult(
    const
    RuntimeServiceCapabilityInvocationPolicyProtocolResult&
        result)
{
    std::string encoded{};

    AppendField(encoded, ProtocolVersion);

    AppendField(
        encoded,
        std::to_string(
            static_cast<std::uint8_t>(
                result.status)));

    AppendField(
        encoded,
        std::to_string(
            result.denied_capability_count));

    AppendField(
        encoded,
        std::to_string(
            result.denied_operation_count));

    AppendField(
        encoded,
        std::to_string(
            result.denied_provider_count));

    AppendField(
        encoded,
        result.diagnostic);

    return encoded;
}

std::optional<
    RuntimeServiceCapabilityInvocationPolicyProtocolResult>
RuntimeServiceCapabilityInvocationPolicyProtocol::
DecodeResult(const std::string_view payload)
{
    std::size_t offset{0U};

    std::string version{};
    std::string status{};
    std::string denied_capability_count{};
    std::string denied_operation_count{};
    std::string denied_provider_count{};

    RuntimeServiceCapabilityInvocationPolicyProtocolResult
        result{};

    if (!ReadField(payload, offset, version) ||
        version != ProtocolVersion ||
        !ReadField(payload, offset, status) ||
        !ReadField(
            payload,
            offset,
            denied_capability_count) ||
        !ReadField(
            payload,
            offset,
            denied_operation_count) ||
        !ReadField(
            payload,
            offset,
            denied_provider_count) ||
        !ReadField(
            payload,
            offset,
            result.diagnostic) ||
        offset != payload.size())
    {
        return std::nullopt;
    }

    std::uint32_t raw_status{0U};

    if (!ParseInteger(status, raw_status) ||
        !ParseInteger(
            denied_capability_count,
            result.denied_capability_count) ||
        !ParseInteger(
            denied_operation_count,
            result.denied_operation_count) ||
        !ParseInteger(
            denied_provider_count,
            result.denied_provider_count) ||
        raw_status >
            static_cast<std::uint32_t>(
                RuntimeServiceCapabilityInvocationPolicyProtocolStatus::
                    UnknownCommand))
    {
        return std::nullopt;
    }

    result.status =
        static_cast<
            RuntimeServiceCapabilityInvocationPolicyProtocolStatus>(
                raw_status);

    return result;
}

} // namespace sovereign
