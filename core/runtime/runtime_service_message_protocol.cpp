#include "runtime_service_message_protocol.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace sovereign
{
namespace
{

void AppendField(
    std::string& output,
    const std::string_view value)
{
    output += std::to_string(value.size());
    output += ':';
    output.append(value);
}

bool ReadField(
    std::string_view input,
    std::size_t& offset,
    std::string& value)
{
    if (offset >= input.size())
    {
        return false;
    }

    const std::size_t delimiter =
        input.find(':', offset);

    if (delimiter == std::string_view::npos ||
        delimiter == offset)
    {
        return false;
    }

    std::size_t length = 0U;

    const char* const begin =
        input.data() + offset;

    const char* const end =
        input.data() + delimiter;

    const auto conversion =
        std::from_chars(begin, end, length);

    if (conversion.ec != std::errc{} ||
        conversion.ptr != end)
    {
        return false;
    }

    offset = delimiter + 1U;

    if (length > input.size() - offset)
    {
        return false;
    }

    value.assign(
        input.substr(offset, length));

    offset += length;
    return true;
}

template<typename Integer>
bool ParseInteger(
    const std::string_view text,
    Integer& value)
{
    static_assert(std::is_integral_v<Integer>);

    if (text.empty())
    {
        return false;
    }

    const char* const begin = text.data();
    const char* const end =
        text.data() + text.size();

    const auto conversion =
        std::from_chars(begin, end, value);

    return
        conversion.ec == std::errc{} &&
        conversion.ptr == end;
}

} // namespace

std::string
RuntimeServiceMessageProtocol::EncodeRequest(
    const RuntimeServiceMessageRequest& request)
{
    std::string encoded{};

    AppendField(encoded, request.service_name);
    AppendField(encoded, request.operation);
    AppendField(encoded, request.payload);

    return encoded;
}

std::optional<RuntimeServiceMessageRequest>
RuntimeServiceMessageProtocol::DecodeRequest(
    const std::string_view payload)
{
    RuntimeServiceMessageRequest request{};
    std::size_t offset = 0U;

    if (!ReadField(
            payload,
            offset,
            request.service_name) ||
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
RuntimeServiceMessageProtocol::EncodeResult(
    const RuntimeServiceInvocationResult& result)
{
    std::string encoded{};

    using StatusValue =
        std::underlying_type_t<
            RuntimeServiceInvocationStatus>;

    AppendField(
        encoded,
        std::to_string(
            static_cast<StatusValue>(
                result.status)));

    AppendField(encoded, result.service_name);
    AppendField(encoded, result.operation);
    AppendField(encoded, result.response_payload);
    AppendField(encoded, result.diagnostic);

    AppendField(
        encoded,
        std::to_string(
            result.invocation_sequence));

    return encoded;
}

std::optional<RuntimeServiceInvocationResult>
RuntimeServiceMessageProtocol::DecodeResult(
    const std::string_view payload)
{
    std::size_t offset = 0U;

    std::string status_text{};
    std::string service_name{};
    std::string operation{};
    std::string response_payload{};
    std::string diagnostic{};
    std::string sequence_text{};

    if (!ReadField(payload, offset, status_text) ||
        !ReadField(payload, offset, service_name) ||
        !ReadField(payload, offset, operation) ||
        !ReadField(
            payload,
            offset,
            response_payload) ||
        !ReadField(payload, offset, diagnostic) ||
        !ReadField(
            payload,
            offset,
            sequence_text) ||
        offset != payload.size())
    {
        return std::nullopt;
    }

    using StatusValue =
        std::underlying_type_t<
            RuntimeServiceInvocationStatus>;

    StatusValue status_value{};
    std::uint64_t sequence = 0U;

    if (!ParseInteger(status_text, status_value) ||
        !ParseInteger(sequence_text, sequence))
    {
        return std::nullopt;
    }

    const auto minimum =
        static_cast<StatusValue>(
            RuntimeServiceInvocationStatus::
                NoAttempt);

    const auto maximum =
        static_cast<StatusValue>(
            RuntimeServiceInvocationStatus::
                HandlerException);

    if (status_value < minimum ||
        status_value > maximum)
    {
        return std::nullopt;
    }

    return RuntimeServiceInvocationResult{
        .status =
            static_cast<
                RuntimeServiceInvocationStatus>(
                    status_value),
        .service_name = std::move(service_name),
        .operation = std::move(operation),
        .response_payload =
            std::move(response_payload),
        .diagnostic = std::move(diagnostic),
        .invocation_sequence = sequence};
}

} // namespace sovereign
