#pragma once

#include "runtime_service_capability_invoker.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sovereign
{

enum class
RuntimeServiceCapabilityInvocationPolicyStatus :
    std::uint8_t
{
    NoEvaluation = 0U,
    Allowed,
    EmptyCapability,
    CapabilityDenied,
    OperationDenied,
    ProviderDenied
};

struct
RuntimeServiceCapabilityInvocationPolicyDecision final
{
    RuntimeServiceCapabilityInvocationPolicyStatus
        status{
            RuntimeServiceCapabilityInvocationPolicyStatus::
                NoEvaluation};

    std::string capability{};
    std::string operation{};
    std::string provider_service_name{};
    std::string diagnostic{};

    [[nodiscard]]
    bool Allowed() const noexcept
    {
        return
            status ==
            RuntimeServiceCapabilityInvocationPolicyStatus::
                Allowed;
    }
};

struct
RuntimeServiceCapabilityOperationPolicyRule final
{
    std::string capability{};
    std::string operation{};
};

struct
RuntimeServiceCapabilityProviderPolicyRule final
{
    std::string capability{};
    std::string provider_service_name{};
};

class RuntimeServiceCapabilityInvocationPolicy final
{
public:
    [[nodiscard]]
    bool DenyCapability(std::string capability);

    [[nodiscard]]
    bool DenyOperation(
        std::string capability,
        std::string operation);

    [[nodiscard]]
    bool DenyProvider(
        std::string capability,
        std::string provider_service_name);

    [[nodiscard]]
    RuntimeServiceCapabilityInvocationPolicyDecision
    EvaluateRequest(
        const RuntimeServiceCapabilityInvocationRequest&
            request) const;

    [[nodiscard]]
    RuntimeServiceCapabilityInvocationPolicyDecision
    EvaluateProvider(
        const RuntimeServiceCapabilityInvocationRequest&
            request,
        std::string_view provider_service_name) const;

    [[nodiscard]]
    std::size_t DeniedCapabilityCount()
        const noexcept;

    [[nodiscard]]
    std::size_t DeniedOperationCount()
        const noexcept;

    [[nodiscard]]
    std::size_t DeniedProviderCount()
        const noexcept;

private:
    std::vector<std::string> denied_capabilities_{};

    std::vector<
        RuntimeServiceCapabilityOperationPolicyRule>
            denied_operations_{};

    std::vector<
        RuntimeServiceCapabilityProviderPolicyRule>
            denied_providers_{};
};

} // namespace sovereign
