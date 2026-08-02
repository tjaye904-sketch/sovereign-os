#include "runtime_service_capability_provider_selection_policy_protocol.hpp"

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

constexpr std::string_view ProtocolVersion{"1"};

constexpr std::string_view SetRankCommand{
    "SET_RANK"};

constexpr std::string_view RemoveRankCommand{
    "REMOVE_RANK"};

constexpr std::string_view SetFallbackCommand{
    "SET_FALLBACK"};

constexpr std::string_view QueryCommand{
    "QUERY"};

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

    if (length >
        encoded.size() - value_offset)
    {
        return false;
    }

    value.assign(
        encoded.substr(
            value_offset,
            length));

    offset = value_offset + length;
    return true;
}

template<typename Integer>
bool ParseInteger(
    const std::string_view value,
    Integer& result)
{
    if (value.empty())
    {
        return false;
    }

    const char* begin = value.data();
    const char* end = begin + value.size();

    const auto parsed =
        std::from_chars(begin, end, result);

    return
        parsed.ec == std::errc{} &&
        parsed.ptr == end;
}

std::string_view EncodeCommand(
    const
    RuntimeServiceCapabilityProviderSelectionPolicyCommand
        command)
{
    switch (command)
    {
        case
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                SetRank:
            return SetRankCommand;

        case
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                RemoveRank:
            return RemoveRankCommand;

        case
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                SetFallback:
            return SetFallbackCommand;

        case
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                Query:
            return QueryCommand;
    }

    return {};
}

bool DecodeCommand(
    const std::string_view encoded,
    RuntimeServiceCapabilityProviderSelectionPolicyCommand&
        command)
{
    if (encoded == SetRankCommand)
    {
        command =
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                SetRank;

        return true;
    }

    if (encoded == RemoveRankCommand)
    {
        command =
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                RemoveRank;

        return true;
    }

    if (encoded == SetFallbackCommand)
    {
        command =
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                SetFallback;

        return true;
    }

    if (encoded == QueryCommand)
    {
        command =
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                Query;

        return true;
    }

    return false;
}

bool ValidRequest(
    const
    RuntimeServiceCapabilityProviderSelectionPolicyProtocolRequest&
        request)
{
    switch (request.command)
    {
        case
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                SetRank:
            return
                !request.capability.empty() &&
                !request.service_name.empty();

        case
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                RemoveRank:
            return
                !request.capability.empty() &&
                !request.service_name.empty();

        case
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                SetFallback:
            return
                request.capability.empty() &&
                request.service_name.empty() &&
                request.rank == 0U;

        case
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                Query:
            return
                request.capability.empty() &&
                request.service_name.empty() &&
                request.rank == 0U &&
                !request
                    .preferred_provider_fallback_enabled;
    }

    return false;
}

} // namespace

std::string
RuntimeServiceCapabilityProviderSelectionPolicyProtocol::
EncodeRequest(
    const
    RuntimeServiceCapabilityProviderSelectionPolicyProtocolRequest&
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
        request.service_name);

    AppendField(
        encoded,
        std::to_string(request.rank));

    AppendField(
        encoded,
        request
                .preferred_provider_fallback_enabled
            ? "1"
            : "0");

    return encoded;
}

std::optional<
    RuntimeServiceCapabilityProviderSelectionPolicyProtocolRequest>
RuntimeServiceCapabilityProviderSelectionPolicyProtocol::
DecodeRequest(
    const std::string_view payload)
{
    std::size_t offset{0U};

    std::string version{};
    std::string command{};
    std::string rank{};
    std::string fallback_enabled{};

    RuntimeServiceCapabilityProviderSelectionPolicyProtocolRequest
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
            request.service_name) ||
        !ReadField(payload, offset, rank) ||
        !ReadField(
            payload,
            offset,
            fallback_enabled) ||
        offset != payload.size() ||
        !DecodeCommand(
            command,
            request.command) ||
        !ParseInteger(rank, request.rank) ||
        (fallback_enabled != "0" &&
         fallback_enabled != "1"))
    {
        return std::nullopt;
    }

    request.preferred_provider_fallback_enabled =
        fallback_enabled == "1";

    if (!ValidRequest(request))
    {
        return std::nullopt;
    }

    return request;
}

std::string
RuntimeServiceCapabilityProviderSelectionPolicyProtocol::
EncodeResult(
    const
    RuntimeServiceCapabilityProviderSelectionPolicyProtocolResult&
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
            result.provider_ranks.size()));

    AppendField(
        encoded,
        std::to_string(
            result.selection_count));

    AppendField(
        encoded,
        std::to_string(
            result.ranked_selection_count));

    AppendField(
        encoded,
        std::to_string(
            result.preferred_selection_count));

    AppendField(
        encoded,
        std::to_string(
            result.preferred_fallback_count));

    AppendField(
        encoded,
        result
                .preferred_provider_fallback_enabled
            ? "1"
            : "0");

    for (const auto& provider_rank :
         result.provider_ranks)
    {
        AppendField(
            encoded,
            provider_rank.capability);

        AppendField(
            encoded,
            provider_rank.service_name);

        AppendField(
            encoded,
            std::to_string(
                provider_rank.rank));
    }

    AppendField(
        encoded,
        result.diagnostic);

    return encoded;
}

std::optional<
    RuntimeServiceCapabilityProviderSelectionPolicyProtocolResult>
RuntimeServiceCapabilityProviderSelectionPolicyProtocol::
DecodeResult(
    const std::string_view payload)
{
    std::size_t offset{0U};

    std::string version{};
    std::string status{};
    std::string provider_rank_count{};
    std::string selection_count{};
    std::string ranked_selection_count{};
    std::string preferred_selection_count{};
    std::string preferred_fallback_count{};
    std::string fallback_enabled{};

    RuntimeServiceCapabilityProviderSelectionPolicyProtocolResult
        result{};

    if (!ReadField(payload, offset, version) ||
        version != ProtocolVersion ||
        !ReadField(payload, offset, status) ||
        !ReadField(
            payload,
            offset,
            provider_rank_count) ||
        !ReadField(
            payload,
            offset,
            selection_count) ||
        !ReadField(
            payload,
            offset,
            ranked_selection_count) ||
        !ReadField(
            payload,
            offset,
            preferred_selection_count) ||
        !ReadField(
            payload,
            offset,
            preferred_fallback_count) ||
        !ReadField(
            payload,
            offset,
            fallback_enabled) ||
        (fallback_enabled != "0" &&
         fallback_enabled != "1"))
    {
        return std::nullopt;
    }

    std::uint32_t raw_status{0U};
    std::size_t rank_count{0U};

    if (!ParseInteger(status, raw_status) ||
        !ParseInteger(
            provider_rank_count,
            rank_count) ||
        !ParseInteger(
            selection_count,
            result.selection_count) ||
        !ParseInteger(
            ranked_selection_count,
            result.ranked_selection_count) ||
        !ParseInteger(
            preferred_selection_count,
            result.preferred_selection_count) ||
        !ParseInteger(
            preferred_fallback_count,
            result.preferred_fallback_count) ||
        raw_status >
            static_cast<std::uint32_t>(
                RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus::
                    UnknownCommand))
    {
        return std::nullopt;
    }

    result.status =
        static_cast<
            RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus>(
                raw_status);

    result.preferred_provider_fallback_enabled =
        fallback_enabled == "1";

    result.provider_ranks.reserve(rank_count);

    for (std::size_t index = 0U;
         index < rank_count;
         ++index)
    {
        RuntimeServiceCapabilityProviderRankEntry
            provider_rank{};

        std::string rank{};

        if (!ReadField(
                payload,
                offset,
                provider_rank.capability) ||
            !ReadField(
                payload,
                offset,
                provider_rank.service_name) ||
            !ReadField(
                payload,
                offset,
                rank) ||
            !ParseInteger(
                rank,
                provider_rank.rank))
        {
            return std::nullopt;
        }

        result.provider_ranks.push_back(
            std::move(provider_rank));
    }

    if (!ReadField(
            payload,
            offset,
            result.diagnostic) ||
        offset != payload.size())
    {
        return std::nullopt;
    }

    return result;
}

} // namespace sovereign
