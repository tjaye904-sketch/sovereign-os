#include "runtime_service_endpoint_protocol.hpp"

#include "runtime_service_message_protocol.hpp"
#include "runtime_service_routed_message_protocol.hpp"
#include "runtime_service_capability_protocol.hpp"
#include "runtime_service_capability_invocation_protocol.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

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

std::string TargetValue(
    const RuntimeServiceEndpointTarget target)
{
    return std::to_string(
        static_cast<std::uint8_t>(target));
}

std::optional<RuntimeServiceEndpointTarget>
DecodeTarget(const std::string_view value)
{
    std::uint32_t raw_target{0U};

    const char* begin = value.data();
    const char* end = begin + value.size();

    const auto parsed =
        std::from_chars(
            begin,
            end,
            raw_target);

    if (parsed.ec != std::errc{} ||
        parsed.ptr != end)
    {
        return std::nullopt;
    }

    switch (raw_target)
    {
        case static_cast<std::uint32_t>(
            RuntimeServiceEndpointTarget::Direct):
            return
                RuntimeServiceEndpointTarget::
                    Direct;

        case static_cast<std::uint32_t>(
            RuntimeServiceEndpointTarget::Routed):
            return
                RuntimeServiceEndpointTarget::
                    Routed;

        case static_cast<std::uint32_t>(
            RuntimeServiceEndpointTarget::
                Capability):
            return
                RuntimeServiceEndpointTarget::
                    Capability;

        case static_cast<std::uint32_t>(
            RuntimeServiceEndpointTarget::
                CapabilityInvocation):
            return
                RuntimeServiceEndpointTarget::
                    CapabilityInvocation;

        default:
            return std::nullopt;
    }
}

} // namespace

bool RuntimeServiceEndpointResult::Completed()
    const noexcept
{
    if (target ==
        RuntimeServiceEndpointTarget::Direct)
    {
        const auto* invocation =
            std::get_if<
                RuntimeServiceInvocationResult>(
                    &result);

        return
            invocation != nullptr &&
            invocation->Completed();
    }

    if (target ==
        RuntimeServiceEndpointTarget::Routed)
    {
        const auto* dispatch =
            std::get_if<
                RuntimeServiceDispatchResult>(
                    &result);

        return
            dispatch != nullptr &&
            dispatch->Completed();
    }

    if (target ==
        RuntimeServiceEndpointTarget::Capability)
    {
        const auto* capability =
            std::get_if<
                RuntimeServiceCapabilityResult>(
                    &result);

        return
            capability != nullptr &&
            capability->Accepted();
    }

    const auto* capability_invocation =
        std::get_if<
            RuntimeServiceCapabilityInvocationResult>(
                &result);

    return
        capability_invocation != nullptr &&
        capability_invocation->Completed();
}

std::string
RuntimeServiceEndpointProtocol::EncodeRequest(
    const RuntimeServiceEndpointRequest& request)
{
    std::string encoded{};

    encoded.reserve(
        request.target_name.size() +
        request.operation.size() +
        request.payload.size() +
        48U);

    AppendField(encoded, ProtocolVersion);
    AppendField(encoded, TargetValue(request.target));
    AppendField(encoded, request.target_name);
    AppendField(encoded, request.operation);
    AppendField(encoded, request.payload);

    return encoded;
}

std::optional<RuntimeServiceEndpointRequest>
RuntimeServiceEndpointProtocol::DecodeRequest(
    const std::string_view payload)
{
    std::size_t offset{0U};

    std::string version{};
    std::string target_value{};

    RuntimeServiceEndpointRequest request{};

    if (!ReadField(payload, offset, version) ||
        version != ProtocolVersion ||
        !ReadField(
            payload,
            offset,
            target_value) ||
        !ReadField(
            payload,
            offset,
            request.target_name) ||
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

    const auto target =
        DecodeTarget(target_value);

    if (!target.has_value())
    {
        return std::nullopt;
    }

    request.target = *target;
    return request;
}

std::string
RuntimeServiceEndpointProtocol::EncodeResult(
    const RuntimeServiceEndpointResult& result)
{
    std::string nested{};

    if (result.target ==
        RuntimeServiceEndpointTarget::Direct)
    {
        const auto* invocation =
            std::get_if<
                RuntimeServiceInvocationResult>(
                    &result.result);

        if (invocation == nullptr)
        {
            return {};
        }

        nested =
            RuntimeServiceMessageProtocol::
                EncodeResult(*invocation);
    }
    else if (
        result.target ==
        RuntimeServiceEndpointTarget::Routed)
    {
        const auto* dispatch =
            std::get_if<
                RuntimeServiceDispatchResult>(
                    &result.result);

        if (dispatch == nullptr)
        {
            return {};
        }

        nested =
            RuntimeServiceRoutedMessageProtocol::
                EncodeResult(*dispatch);
    }
    else if (
        result.target ==
        RuntimeServiceEndpointTarget::Capability)
    {
        const auto* capability =
            std::get_if<
                RuntimeServiceCapabilityResult>(
                    &result.result);

        if (capability == nullptr)
        {
            return {};
        }

        nested =
            RuntimeServiceCapabilityProtocol::
                EncodeResult(*capability);
    }
    else
    {
        const auto* capability_invocation =
            std::get_if<
                RuntimeServiceCapabilityInvocationResult>(
                    &result.result);

        if (capability_invocation == nullptr)
        {
            return {};
        }

        nested =
            RuntimeServiceCapabilityInvocationProtocol::
                EncodeResult(*capability_invocation);
    }

    std::string encoded{};
    encoded.reserve(nested.size() + 32U);

    AppendField(encoded, ProtocolVersion);
    AppendField(encoded, TargetValue(result.target));
    AppendField(encoded, nested);

    return encoded;
}

std::optional<RuntimeServiceEndpointResult>
RuntimeServiceEndpointProtocol::DecodeResult(
    const std::string_view payload)
{
    std::size_t offset{0U};

    std::string version{};
    std::string target_value{};
    std::string nested{};

    if (!ReadField(payload, offset, version) ||
        version != ProtocolVersion ||
        !ReadField(
            payload,
            offset,
            target_value) ||
        !ReadField(
            payload,
            offset,
            nested) ||
        offset != payload.size())
    {
        return std::nullopt;
    }

    const auto target =
        DecodeTarget(target_value);

    if (!target.has_value())
    {
        return std::nullopt;
    }

    RuntimeServiceEndpointResult result{};
    result.target = *target;

    if (*target ==
        RuntimeServiceEndpointTarget::Direct)
    {
        const auto invocation =
            RuntimeServiceMessageProtocol::
                DecodeResult(nested);

        if (!invocation.has_value())
        {
            return std::nullopt;
        }

        result.result = *invocation;
    }
    else if (
        *target ==
        RuntimeServiceEndpointTarget::Routed)
    {
        const auto dispatch =
            RuntimeServiceRoutedMessageProtocol::
                DecodeResult(nested);

        if (!dispatch.has_value())
        {
            return std::nullopt;
        }

        result.result = *dispatch;
    }
    else if (
        *target ==
        RuntimeServiceEndpointTarget::Capability)
    {
        const auto capability =
            RuntimeServiceCapabilityProtocol::
                DecodeResult(nested);

        if (!capability.has_value())
        {
            return std::nullopt;
        }

        result.result = *capability;
    }
    else
    {
        const auto capability_invocation =
            RuntimeServiceCapabilityInvocationProtocol::
                DecodeResult(nested);

        if (!capability_invocation.has_value())
        {
            return std::nullopt;
        }

        result.result = *capability_invocation;
    }

    return result;
}

} // namespace sovereign
