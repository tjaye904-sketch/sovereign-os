#pragma once

#include "service_capability.hpp"
#include "service_registry.hpp"
#include "service_state.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace sovereign
{

class
RuntimeServiceCapabilityProviderSelectionPolicy;

enum class RuntimeServiceCapabilityStatus :
    std::uint8_t
{
    NoAttempt = 0U,
    Accepted,
    EmptyCapability,
    CapabilityNotFound,
    NoEligibleProvider,
    PreferredProviderUnavailable,
    NegotiatorUnavailable
};

struct RuntimeServiceCapabilityRequest final
{
    std::string capability{};
    std::string preferred_service_name{};
    bool require_running{true};
};

struct RuntimeServiceCapabilityResult final
{
    RuntimeServiceCapabilityStatus status{
        RuntimeServiceCapabilityStatus::NoAttempt};

    std::string capability{};
    std::string selected_service_name{};

    ServiceState selected_service_state{
        ServiceState::Created};

    std::size_t selected_service_ordinal{0U};
    std::size_t candidate_count{0U};

    std::uint64_t registry_generation{0U};
    std::uint64_t negotiation_sequence{0U};

    std::string diagnostic{};

    [[nodiscard]]
    bool Accepted() const noexcept
    {
        return
            status ==
            RuntimeServiceCapabilityStatus::Accepted;
    }
};

class RuntimeServiceCapabilityNegotiator final
{
public:
    explicit RuntimeServiceCapabilityNegotiator(
        const ServiceRegistry& registry) noexcept;

    RuntimeServiceCapabilityNegotiator(
        const ServiceRegistry& registry,
        RuntimeServiceCapabilityProviderSelectionPolicy&
            selection_policy) noexcept;

    [[nodiscard]]
    RuntimeServiceCapabilityResult Negotiate(
        RuntimeServiceCapabilityRequest request);

    [[nodiscard]]
    const RuntimeServiceCapabilityResult&
    LastNegotiation() const noexcept;

    [[nodiscard]]
    std::uint64_t NegotiationCount() const noexcept;

    [[nodiscard]]
    std::uint64_t AcceptedCount() const noexcept;

private:
    const ServiceRegistry& registry_;
    ServiceCapabilityDiscovery discovery_;

    RuntimeServiceCapabilityProviderSelectionPolicy*
        selection_policy_{nullptr};

    RuntimeServiceCapabilityResult
        last_negotiation_{};

    std::uint64_t negotiation_count_{0U};
    std::uint64_t accepted_count_{0U};
};

} // namespace sovereign
