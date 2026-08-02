#include "runtime_service_routed_message_protocol.hpp"

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace sovereign
{
namespace
{

void AppendField(
    std::string& encoded,
    const std::string_view value)
{
    encoded.append(std::to_string(value.size()));
    encoded.push_back(':');
    encoded.append(value);
}

void AppendUnsigned(
    std::string& encoded,
    const std::uint64_t value)
{
    AppendField(encoded, std::to_string(value));
}

bool ReadField(
    std::string_view encoded,
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

    if (length >
        encoded.size() - value_offset)
    {
        return false;
    }

    value.assign(
        encoded.substr(value_offset, length));

    offset = value_offset + length;
    return true;
}

bool ReadUnsigned(
    std::string_view encoded,
    std::size_t& offset,
    std::uint64_t& value)
{
    std::string field{};

    if (!ReadField(encoded, offset, field) ||
        field.empty())
    {
        return false;
    }

    const char* begin = field.data();
    const char* end = begin + field.size();

    const auto parsed =
        std::from_chars(begin, end, value);

    return
        parsed.ec == std::errc{} &&
        parsed.ptr == end;
}

bool ReadStatus(
    std::string_view encoded,
    std::size_t& offset,
    RuntimeServiceDispatchStatus& status)
{
    std::uint64_t raw_status{0U};

    if (!ReadUnsigned(
            encoded,
            offset,
            raw_status))
    {
        return false;
    }

    constexpr auto maximum =
        static_cast<std::uint64_t>(
            RuntimeServiceDispatchStatus::
                InvocationFailed);

    if (raw_status > maximum)
    {
        return false;
    }

    status =
        static_cast<
            RuntimeServiceDispatchStatus>(
                raw_status);

    return true;
}

bool ReadInvocationStatus(
    std::string_view encoded,
    std::size_t& offset,
    RuntimeServiceInvocationStatus& status)
{
    std::uint64_t raw_status{0U};

    if (!ReadUnsigned(
            encoded,
            offset,
            raw_status))
    {
        return false;
    }

    constexpr auto maximum =
        static_cast<std::uint64_t>(
            RuntimeServiceInvocationStatus::
                OperationRejected);

    if (raw_status > maximum)
    {
        return false;
    }

    status =
        static_cast<
            RuntimeServiceInvocationStatus>(
                raw_status);

    return true;
}

} // namespace

std::string
RuntimeServiceRoutedMessageProtocol::EncodeRequest(
    const RuntimeServiceRoutedMessageRequest& request)
{
    std::string encoded{};
    encoded.reserve(
        request.route_name.size() +
        request.operation.size() +
        request.payload.size() +
        48U);

    AppendField(encoded, request.route_name);
    AppendField(encoded, request.operation);
    AppendField(encoded, request.payload);

    return encoded;
}

std::optional<RuntimeServiceRoutedMessageRequest>
RuntimeServiceRoutedMessageProtocol::DecodeRequest(
    const std::string_view payload)
{
    RuntimeServiceRoutedMessageRequest request{};
    std::size_t offset{0U};

    if (!ReadField(
            payload,
            offset,
            request.route_name) ||
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
RuntimeServiceRoutedMessageProtocol::EncodeResult(
    const RuntimeServiceDispatchResult& result)
{
    std::string encoded{};

    encoded.reserve(
        result.route_name.size() +
        result.service_name.size() +
        result.operation.size() +
        result.diagnostic.size() +
        result.invocation.service_name.size() +
        result.invocation.operation.size() +
        result.invocation.response_payload.size() +
        result.invocation.diagnostic.size() +
        160U);

    AppendUnsigned(
        encoded,
        static_cast<std::uint64_t>(
            result.status));

    AppendField(encoded, result.route_name);
    AppendField(encoded, result.service_name);
    AppendField(encoded, result.operation);
    AppendField(encoded, result.diagnostic);

    AppendUnsigned(
        encoded,
        result.dispatch_sequence);

    AppendUnsigned(
        encoded,
        static_cast<std::uint64_t>(
            result.invocation.status));

    AppendField(
        encoded,
        result.invocation.service_name);

    AppendField(
        encoded,
        result.invocation.operation);

    AppendField(
        encoded,
        result.invocation.response_payload);

    AppendField(
        encoded,
        result.invocation.diagnostic);

    AppendUnsigned(
        encoded,
        result.invocation.invocation_sequence);

    return encoded;
}

std::optional<RuntimeServiceDispatchResult>
RuntimeServiceRoutedMessageProtocol::DecodeResult(
    const std::string_view payload)
{
    RuntimeServiceDispatchResult result{};
    std::size_t offset{0U};

    if (!ReadStatus(
            payload,
            offset,
            result.status) ||
        !ReadField(
            payload,
            offset,
            result.route_name) ||
        !ReadField(
            payload,
            offset,
            result.service_name) ||
        !ReadField(
            payload,
            offset,
            result.operation) ||
        !ReadField(
            payload,
            offset,
            result.diagnostic) ||
        !ReadUnsigned(
            payload,
            offset,
            result.dispatch_sequence) ||
        !ReadInvocationStatus(
            payload,
            offset,
            result.invocation.status) ||
        !ReadField(
            payload,
            offset,
            result.invocation.service_name) ||
        !ReadField(
            payload,
            offset,
            result.invocation.operation) ||
        !ReadField(
            payload,
            offset,
            result.invocation.response_payload) ||
        !ReadField(
            payload,
            offset,
            result.invocation.diagnostic) ||
        !ReadUnsigned(
            payload,
            offset,
            result.invocation.invocation_sequence) ||
        offset != payload.size())
    {
        return std::nullopt;
    }

    return result;
}

} // namespace sovereign
