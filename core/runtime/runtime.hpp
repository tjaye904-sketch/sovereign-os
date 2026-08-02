#pragma once

#include "event_bus.hpp"
#include "lifecycle_manager.hpp"
#include "runtime_message_bus.hpp"
#include "runtime_service_capability_invocation_policy.hpp"
#include "runtime_service_capability_invoker.hpp"
#include "runtime_service_capability_negotiator.hpp"
#include "runtime_service_capability_provider_selection_policy.hpp"
#include "runtime_service_dispatcher.hpp"
#include "runtime_service_endpoint.hpp"
#include "runtime_service_invoker.hpp"
#include "runtime_service_route.hpp"
#include "scheduler.hpp"
#include "service_host.hpp"
#include "service_registry.hpp"
#include "runtime_service_capability_trace_query.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>

namespace sovereign
{

class Runtime final
{
public:
    Runtime();

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    Runtime(Runtime&&) = delete;
    Runtime& operator=(Runtime&&) = delete;

    bool RegisterService(
        std::unique_ptr<Service> service);

    void SetMaximumFrames(
        std::uint64_t maximum_frames) noexcept;

    bool Start();

    bool Run();

    bool Stop();

    [[nodiscard]]
    RuntimeState State() const noexcept;

    [[nodiscard]]
    std::uint64_t FrameCount() const noexcept;

    [[nodiscard]]
    EventBus& Events() noexcept;

    [[nodiscard]]
    RuntimeMessageBus& Messaging() noexcept;

    [[nodiscard]]
    const RuntimeMessageBus&
    Messaging() const noexcept;

    [[nodiscard]]
    ServiceRegistry& Services() noexcept;

    [[nodiscard]]
    const ServiceRegistry&
    Services() const noexcept;

    [[nodiscard]]
    RuntimeServiceEndpoint&
    ServiceEndpoint() noexcept;

    [[nodiscard]]
    const RuntimeServiceEndpoint&
    ServiceEndpoint() const noexcept;

    [[nodiscard]]
    RuntimeServiceCapabilityProviderSelectionPolicy&
    CapabilityProviderSelectionPolicy() noexcept;

    [[nodiscard]]
    const
    RuntimeServiceCapabilityProviderSelectionPolicy&
    CapabilityProviderSelectionPolicy()
        const noexcept;

    [[nodiscard]]
    RuntimeServiceCapabilityInvocationPolicy&
    CapabilityInvocationPolicy() noexcept;

    [[nodiscard]]
    const RuntimeServiceCapabilityInvocationPolicy&
    CapabilityInvocationPolicy() const noexcept;

    [[nodiscard]]
    const RuntimeServiceCapabilityPipelineTrace&
    LastCapabilityPipelineTrace() const noexcept;

    [[nodiscard]]
    const std::deque<
        RuntimeServiceCapabilityPipelineTrace>&
    CapabilityPipelineTraces() const noexcept;

    [[nodiscard]]
    std::size_t CapabilityPipelineTraceCount()
        const noexcept;

    void SetCapabilityPipelineTraceCapacity(
        std::size_t capacity) noexcept;

    [[nodiscard]]
    std::size_t CapabilityPipelineTraceCapacity()
        const noexcept;

    [[nodiscard]]
    std::uint64_t
    DiscardedCapabilityPipelineTraceCount()
        const noexcept;

    [[nodiscard]]
    RuntimeServiceCapabilityTraceQueryResult
    QueryCapabilityPipelineTraces(
        const RuntimeServiceCapabilityTraceQuery&
            query) const;

    [[nodiscard]]
    std::optional<
        RuntimeServiceCapabilityPipelineTrace>
    FindCapabilityPipelineTrace(
        const Uuid& trace_id) const;


    void ClearCapabilityPipelineTraces() noexcept;

private:
    EventBus event_bus_;

    RuntimeMessageBus message_bus_;

    /*
     * Policies precede the service registry because
     * registry-owned control services retain
     * references to these objects.
     */
    RuntimeServiceCapabilityProviderSelectionPolicy
        capability_provider_selection_policy_;

    RuntimeServiceCapabilityInvocationPolicy
        capability_invocation_policy_;

    ServiceRegistry service_registry_;

    ServiceHost service_host_;

    Scheduler scheduler_;

    LifecycleManager lifecycle_;

    RuntimeServiceInvoker service_invoker_;

    RuntimeServiceRouteRegistry
        service_route_registry_;

    RuntimeServiceDispatcher service_dispatcher_;

    RuntimeServiceCapabilityNegotiator
        capability_negotiator_;

    RuntimeServiceCapabilityInvoker
        capability_invoker_;

    RuntimeServiceEndpoint service_endpoint_;

    bool built_in_services_registered_{false};
    bool startup_complete_{false};
};

} // namespace sovereign
