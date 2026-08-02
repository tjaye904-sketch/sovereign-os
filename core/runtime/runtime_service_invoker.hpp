#pragma once

#include "runtime_service_invocation.hpp"
#include "service_registry.hpp"

#include <cstdint>

namespace sovereign
{

class RuntimeServiceInvoker final
{
public:
    explicit RuntimeServiceInvoker(
        ServiceRegistry& registry) noexcept;

    [[nodiscard]]
    RuntimeServiceInvocationResult Invoke(
        RuntimeServiceInvocationRequest request);

    [[nodiscard]]
    const RuntimeServiceInvocationResult&
    LastInvocation() const noexcept;

    [[nodiscard]]
    std::uint64_t InvocationCount() const noexcept;

    [[nodiscard]]
    std::uint64_t CompletedCount() const noexcept;

private:
    ServiceRegistry& registry_;

    RuntimeServiceInvocationResult last_invocation_{};

    std::uint64_t invocation_count_{0U};
    std::uint64_t completed_count_{0U};
};

} // namespace sovereign
