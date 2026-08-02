#pragma once

#include "runtime_service_capability_provider_selection_policy.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace sovereign
{

enum class
RuntimeServiceCapabilityProviderSelectionPolicyCommand :
    std::uint8_t
{
    SetRank = 0U,
    RemoveRank,
    SetFallback,
    Query
};

enum class
RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus :
    std::uint8_t
{
    NoAttempt = 0U,
    Accepted,
    InvalidPayload,
    UnsupportedVersion,
    MalformedRequest,
    UnknownCommand
};

struct
RuntimeServiceCapabilityProviderSelectionPolicyProtocolRequest
    final
{
    RuntimeServiceCapabilityProviderSelectionPolicyCommand
        command{
            RuntimeServiceCapabilityProviderSelectionPolicyCommand::
                Query};

    std::string capability{};
    std::string service_name{};

    std::uint32_t rank{0U};

    bool preferred_provider_fallback_enabled{
        false};
};

struct
RuntimeServiceCapabilityProviderSelectionPolicyProtocolResult
    final
{
    RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus
        status{
            RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus::
                NoAttempt};

    std::vector<
        RuntimeServiceCapabilityProviderRankEntry>
            provider_ranks{};

    std::uint64_t selection_count{0U};
    std::uint64_t ranked_selection_count{0U};
    std::uint64_t preferred_selection_count{0U};
    std::uint64_t preferred_fallback_count{0U};

    bool preferred_provider_fallback_enabled{
        false};

    std::string diagnostic{};

    [[nodiscard]]
    bool Accepted() const noexcept
    {
        return
            status ==
            RuntimeServiceCapabilityProviderSelectionPolicyProtocolStatus::
                Accepted;
    }
};

class
RuntimeServiceCapabilityProviderSelectionPolicyProtocol
    final
{
public:
    [[nodiscard]]
    static std::string EncodeRequest(
        const
        RuntimeServiceCapabilityProviderSelectionPolicyProtocolRequest&
            request);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceCapabilityProviderSelectionPolicyProtocolRequest>
    DecodeRequest(std::string_view payload);

    [[nodiscard]]
    static std::string EncodeResult(
        const
        RuntimeServiceCapabilityProviderSelectionPolicyProtocolResult&
            result);

    [[nodiscard]]
    static std::optional<
        RuntimeServiceCapabilityProviderSelectionPolicyProtocolResult>
    DecodeResult(std::string_view payload);

private:
    RuntimeServiceCapabilityProviderSelectionPolicyProtocol() =
        delete;
};

} // namespace sovereign
