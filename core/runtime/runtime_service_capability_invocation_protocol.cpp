#include "runtime_service_capability_invocation_protocol.hpp"

#include "runtime_service_capability_protocol.hpp"
#include "runtime_service_message_protocol.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

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
    const std::string_view value,
    Integer& result)
{
    const char* begin = value.data();
    const char* end = begin + value.size();

    const auto parsed =
        std::from_chars(begin, end, result);

    return
        parsed.ec == std::errc{} &&
        parsed.ptr == end;
}

} // namespace

std::string
RuntimeServiceCapabilityInvocationProtocol::
EncodeRequest(
    const RuntimeServiceCapabilityInvocationRequest&
        request)
{
    std::string encoded{};

    AppendField(encoded, ProtocolVersion);
    AppendField(encoded, request.capability);

    AppendField(
        encoded,
        request.preferred_service_name);

    AppendField(encoded, request.operation);
    AppendField(encoded, request.payload);

    return encoded;
}

std::optional<
    RuntimeServiceCapabilityInvocationRequest>
RuntimeServiceCapabilityInvocationProtocol::
DecodeRequest(const std::string_view payload)
{
    std::size_t offset{0U};

    std::string version{};

    RuntimeServiceCapabilityInvocationRequest
        request{};

    if (!ReadField(payload, offset, version) ||
        version != ProtocolVersion ||
        !ReadField(
            payload,
            offset,
            request.capability) ||
        !ReadField(
            payload,
            offset,
            request.preferred_service_name) ||
        !ReadField(
            payload,
            offset,
            request.operation) ||
        !ReadField(
            payload,
            offset,
            request.payload) ||
        offset != payload.size())
    {
        return std::nullopt;
    }

    return request;
}

std::string
RuntimeServiceCapabilityInvocationProtocol::
EncodeResult(
    const RuntimeServiceCapabilityInvocationResult&
        result)
{
    const std::string negotiation =
        RuntimeServiceCapabilityProtocol::
            EncodeResult(result.negotiation);

    const std::string invocation =
        RuntimeServiceMessageProtocol::
            EncodeResult(result.invocation);

    std::string encoded{};

    AppendField(encoded, ProtocolVersion);

    AppendField(
        encoded,
        std::to_string(
            static_cast<std::uint8_t>(
                result.status)));

    AppendField(encoded, negotiation);
    AppendField(encoded, invocation);

    AppendField(
        encoded,
        std::to_string(result.policy_status));

    AppendField(
        encoded,
        result.policy_capability);

    AppendField(
        encoded,
        result.policy_operation);

    AppendField(
        encoded,
        result.policy_provider_service_name);

    AppendField(
        encoded,
        std::to_string(
            result.capability_invocation_sequence));

    AppendField(encoded, result.diagnostic);

    return encoded;
}

std::optional<
    RuntimeServiceCapabilityInvocationResult>
RuntimeServiceCapabilityInvocationProtocol::
DecodeResult(const std::string_view payload)
{
    std::size_t offset{0U};

    std::string version{};
    std::string status{};
    std::string negotiation_payload{};
    std::string invocation_payload{};
    std::string policy_status{};
    std::string sequence{};

    RuntimeServiceCapabilityInvocationResult
        result{};

    if (!ReadField(payload, offset, version) ||
        version != ProtocolVersion ||
        !ReadField(payload, offset, status) ||
        !ReadField(
            payload,
            offset,
            negotiation_payload) ||
        !ReadField(
            payload,
            offset,
            invocation_payload) ||
        !ReadField(
            payload,
            offset,
            policy_status) ||
        !ReadField(
            payload,
            offset,
            result.policy_capability) ||
        !ReadField(
            payload,
            offset,
            result.policy_operation) ||
        !ReadField(
            payload,
            offset,
            result.policy_provider_service_name) ||
        !ReadField(payload, offset, sequence) ||
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
            policy_status,
            result.policy_status) ||
        !ParseInteger(
            sequence,
            result.capability_invocation_sequence) ||
        raw_status >
            static_cast<std::uint32_t>(
                RuntimeServiceCapabilityInvocationStatus::
                    PolicyRejected))
    {
        return std::nullopt;
    }

    const auto negotiation =
        RuntimeServiceCapabilityProtocol::
            DecodeResult(negotiation_payload);

    const auto invocation =
        RuntimeServiceMessageProtocol::
            DecodeResult(invocation_payload);

    if (!negotiation.has_value() ||
        !invocation.has_value())
    {
        return std::nullopt;
    }

    result.status =
        static_cast<
            RuntimeServiceCapabilityInvocationStatus>(
                raw_status);

    result.negotiation = *negotiation;
    result.invocation = *invocation;

    return result;
}

} // namespace sovereign
