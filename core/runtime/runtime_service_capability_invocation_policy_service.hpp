#pragma once

#include "runtime_service_capability_invocation_policy.hpp"
#include "runtime_service_capability_invocation_policy_protocol.hpp"
#include "runtime_service_invocation.hpp"
#include "service.hpp"

#include <mutex>
#include <string>
#include <string_view>

namespace sovereign
{

class RuntimeServiceCapabilityInvocationPolicyService final
    : public Service,
      public RuntimeInvocableService
{
public:
    static constexpr std::string_view ServiceName{
        "CapabilityInvocationPolicyService"};

    static constexpr std::string_view ApplyOperation{
        "apply"};

    explicit
    RuntimeServiceCapabilityInvocationPolicyService(
        RuntimeServiceCapabilityInvocationPolicy&
            policy) noexcept;

    [[nodiscard]]
    std::string_view Name() const noexcept override;

    bool Start(EventBus& event_bus) override;

    bool Update(
        const FrameContext& frame) override;

    bool Stop() override;

    [[nodiscard]]
    bool SupportsOperation(
        std::string_view operation)
        const noexcept override;

    [[nodiscard]]
    bool Invoke(
        std::string_view operation,
        std::string_view request_payload,
        std::string& response_payload) override;

private:
    [[nodiscard]]
    RuntimeServiceCapabilityInvocationPolicyProtocolResult
    Execute(
        const
        RuntimeServiceCapabilityInvocationPolicyProtocolRequest&
            request);

    [[nodiscard]]
    RuntimeServiceCapabilityInvocationPolicyProtocolResult
    Snapshot(
        RuntimeServiceCapabilityInvocationPolicyProtocolStatus
            status,
        std::string diagnostic) const;

    RuntimeServiceCapabilityInvocationPolicy& policy_;

    mutable std::mutex mutex_{};
};

} // namespace sovereign
