#include "runtime_service_capability_protocol.hpp"

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
RuntimeServiceCapabilityProtocol::EncodeRequest(
    const RuntimeServiceCapabilityRequest& request)
{
    std::string encoded{};

    AppendField(encoded, ProtocolVersion);
    AppendField(encoded, request.capability);
    AppendField(
        encoded,
        request.preferred_service_name);

    AppendField(
        encoded,
        request.require_running ? "1" : "0");

    return encoded;
}

std::optional<RuntimeServiceCapabilityRequest>
RuntimeServiceCapabilityProtocol::DecodeRequest(
    const std::string_view payload)
{
    std::size_t offset{0U};

    std::string version{};
    std::string require_running{};

    RuntimeServiceCapabilityRequest request{};

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
            require_running) ||
        offset != payload.size() ||
        (require_running != "0" &&
         require_running != "1"))
    {
        return std::nullopt;
    }

    request.require_running =
        require_running == "1";

    return request;
}

std::string
RuntimeServiceCapabilityProtocol::EncodeResult(
    const RuntimeServiceCapabilityResult& result)
{
    std::string encoded{};

    AppendField(encoded, ProtocolVersion);

    AppendField(
        encoded,
        std::to_string(
            static_cast<std::uint8_t>(
                result.status)));

    AppendField(encoded, result.capability);

    AppendField(
        encoded,
        result.selected_service_name);

    AppendField(
        encoded,
        std::to_string(
            static_cast<std::uint8_t>(
                result.selected_service_state)));

    AppendField(
        encoded,
        std::to_string(
            result.selected_service_ordinal));

    AppendField(
        encoded,
        std::to_string(result.candidate_count));

    AppendField(
        encoded,
        std::to_string(
            result.registry_generation));

    AppendField(
        encoded,
        std::to_string(
            result.negotiation_sequence));

    AppendField(encoded, result.diagnostic);

    return encoded;
}

std::optional<RuntimeServiceCapabilityResult>
RuntimeServiceCapabilityProtocol::DecodeResult(
    const std::string_view payload)
{
    std::size_t offset{0U};

    std::string version{};
    std::string status{};
    std::string service_state{};
    std::string service_ordinal{};
    std::string candidate_count{};
    std::string registry_generation{};
    std::string negotiation_sequence{};

    RuntimeServiceCapabilityResult result{};

    if (!ReadField(payload, offset, version) ||
        version != ProtocolVersion ||
        !ReadField(payload, offset, status) ||
        !ReadField(
            payload,
            offset,
            result.capability) ||
        !ReadField(
            payload,
            offset,
            result.selected_service_name) ||
        !ReadField(
            payload,
            offset,
            service_state) ||
        !ReadField(
            payload,
            offset,
            service_ordinal) ||
        !ReadField(
            payload,
            offset,
            candidate_count) ||
        !ReadField(
            payload,
            offset,
            registry_generation) ||
        !ReadField(
            payload,
            offset,
            negotiation_sequence) ||
        !ReadField(
            payload,
            offset,
            result.diagnostic) ||
        offset != payload.size())
    {
        return std::nullopt;
    }

    std::uint32_t raw_status{0U};
    std::uint32_t raw_state{0U};

    if (!ParseInteger(status, raw_status) ||
        !ParseInteger(service_state, raw_state) ||
        !ParseInteger(
            service_ordinal,
            result.selected_service_ordinal) ||
        !ParseInteger(
            candidate_count,
            result.candidate_count) ||
        !ParseInteger(
            registry_generation,
            result.registry_generation) ||
        !ParseInteger(
            negotiation_sequence,
            result.negotiation_sequence) ||
        raw_status >
            static_cast<std::uint32_t>(
                RuntimeServiceCapabilityStatus::
                    NegotiatorUnavailable))
    {
        return std::nullopt;
    }

    result.status =
        static_cast<
            RuntimeServiceCapabilityStatus>(
                raw_status);

    result.selected_service_state =
        static_cast<ServiceState>(raw_state);

    return result;
}

} // namespace sovereign
