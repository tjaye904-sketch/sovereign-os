#include "runtime.hpp"

#include "runtime_service_capability_invocation_policy_service.hpp"
#include "runtime_service_capability_provider_selection_policy_service.hpp"

#include <memory>
#include <utility>

namespace sovereign
{

Runtime::Runtime()
    :
    message_bus_(event_bus_),
    service_host_(
        event_bus_,
        service_registry_),
    scheduler_(event_bus_),
    service_invoker_(service_registry_),
    service_dispatcher_(
        service_route_registry_,
        service_invoker_),
    capability_negotiator_(
        service_registry_,
        capability_provider_selection_policy_),
    capability_invoker_(
        capability_negotiator_,
        service_invoker_,
        capability_invocation_policy_),
    service_endpoint_(
        message_bus_,
        service_invoker_,
        service_dispatcher_,
        capability_negotiator_,
        capability_invoker_)
{
    const bool provider_policy_service_registered =
        service_registry_.Register(
            std::make_unique<
                RuntimeServiceCapabilityProviderSelectionPolicyService>(
                    capability_provider_selection_policy_));

    const bool invocation_policy_service_registered =
        service_registry_.Register(
            std::make_unique<
                RuntimeServiceCapabilityInvocationPolicyService>(
                    capability_invocation_policy_));

    built_in_services_registered_ =
        provider_policy_service_registered &&
        invocation_policy_service_registered;

    scheduler_.SetUpdateHandler(
        [this](const FrameContext& frame)
        {
            if (!service_host_.UpdateAll(frame))
            {
                lifecycle_.Transition(
                    RuntimeState::Faulted);

                scheduler_.Stop();
            }
        });
}

bool Runtime::RegisterService(
    std::unique_ptr<Service> service)
{
    if (startup_complete_)
    {
        return false;
    }

    return service_registry_.Register(
        std::move(service));
}

void Runtime::SetMaximumFrames(
    const std::uint64_t maximum_frames) noexcept
{
    scheduler_.SetMaximumFrames(
        maximum_frames);
}

bool Runtime::Start()
{
    if (!built_in_services_registered_)
    {
        lifecycle_.Transition(
            RuntimeState::Faulted);

        return false;
    }

    if (!lifecycle_.Transition(
            RuntimeState::Initializing))
    {
        return false;
    }

    if (!lifecycle_.Transition(
            RuntimeState::Starting))
    {
        lifecycle_.Transition(
            RuntimeState::Faulted);

        return false;
    }

    if (!service_host_.StartAll())
    {
        lifecycle_.Transition(
            RuntimeState::Faulted);

        return false;
    }

    if (!service_endpoint_.Start())
    {
        service_host_.StopAll();

        lifecycle_.Transition(
            RuntimeState::Faulted);

        return false;
    }

    startup_complete_ = true;

    return lifecycle_.Transition(
        RuntimeState::Running);
}

bool Runtime::Run()
{
    if (lifecycle_.State() !=
        RuntimeState::Running)
    {
        return false;
    }

    scheduler_.Run();

    return lifecycle_.State() !=
        RuntimeState::Faulted;
}

bool Runtime::Stop()
{
    const RuntimeState state =
        lifecycle_.State();

    if (state == RuntimeState::Stopped)
    {
        return true;
    }

    if (state != RuntimeState::Running &&
        state != RuntimeState::Paused &&
        state != RuntimeState::Faulted)
    {
        return false;
    }

    scheduler_.Stop();

    if (!lifecycle_.Transition(
            RuntimeState::Stopping))
    {
        return false;
    }

    const bool endpoint_stopped =
        service_endpoint_.Stop();

    const bool services_stopped =
        service_host_.StopAll();

    if (!endpoint_stopped ||
        !services_stopped)
    {
        lifecycle_.Transition(
            RuntimeState::Faulted);

        return false;
    }

    return lifecycle_.Transition(
        RuntimeState::Stopped);
}

RuntimeState Runtime::State() const noexcept
{
    return lifecycle_.State();
}

std::uint64_t Runtime::FrameCount() const noexcept
{
    return scheduler_.FrameCount();
}

EventBus& Runtime::Events() noexcept
{
    return event_bus_;
}

RuntimeMessageBus& Runtime::Messaging() noexcept
{
    return message_bus_;
}

const RuntimeMessageBus&
Runtime::Messaging() const noexcept
{
    return message_bus_;
}

ServiceRegistry& Runtime::Services() noexcept
{
    return service_registry_;
}

const ServiceRegistry&
Runtime::Services() const noexcept
{
    return service_registry_;
}

RuntimeServiceEndpoint&
Runtime::ServiceEndpoint() noexcept
{
    return service_endpoint_;
}

const RuntimeServiceEndpoint&
Runtime::ServiceEndpoint() const noexcept
{
    return service_endpoint_;
}

RuntimeServiceCapabilityProviderSelectionPolicy&
Runtime::CapabilityProviderSelectionPolicy() noexcept
{
    return capability_provider_selection_policy_;
}

const
RuntimeServiceCapabilityProviderSelectionPolicy&
Runtime::CapabilityProviderSelectionPolicy()
    const noexcept
{
    return capability_provider_selection_policy_;
}

RuntimeServiceCapabilityInvocationPolicy&
Runtime::CapabilityInvocationPolicy() noexcept
{
    return capability_invocation_policy_;
}

const RuntimeServiceCapabilityInvocationPolicy&
Runtime::CapabilityInvocationPolicy() const noexcept
{
    return capability_invocation_policy_;
}


const RuntimeServiceCapabilityPipelineTrace&
Runtime::LastCapabilityPipelineTrace()
    const noexcept
{
    return capability_invoker_.LastTrace();
}

const std::deque<
    RuntimeServiceCapabilityPipelineTrace>&
Runtime::CapabilityPipelineTraces()
    const noexcept
{
    return capability_invoker_.Traces();
}

std::size_t
Runtime::CapabilityPipelineTraceCount()
    const noexcept
{
    return capability_invoker_.TraceCount();
}

void Runtime::SetCapabilityPipelineTraceCapacity(
    const std::size_t capacity) noexcept
{
    capability_invoker_.SetTraceCapacity(
        capacity);
}

std::size_t
Runtime::CapabilityPipelineTraceCapacity()
    const noexcept
{
    return capability_invoker_.TraceCapacity();
}

std::uint64_t
Runtime::DiscardedCapabilityPipelineTraceCount()
    const noexcept
{
    return capability_invoker_
        .DiscardedTraceCount();
}

void Runtime::ClearCapabilityPipelineTraces()
    noexcept
{
    capability_invoker_.ClearTraces();
}

} // namespace sovereign

namespace sovereign
{

RuntimeServiceCapabilityTraceQueryResult
Runtime::QueryCapabilityPipelineTraces(
    const RuntimeServiceCapabilityTraceQuery&
        query) const
{
    return QueryRuntimeServiceCapabilityTraces(
        CapabilityPipelineTraces(),
        query);
}

std::optional<
    RuntimeServiceCapabilityPipelineTrace>
Runtime::FindCapabilityPipelineTrace(
    const Uuid& trace_id) const
{
    return FindRuntimeServiceCapabilityTrace(
        CapabilityPipelineTraces(),
        trace_id);
}

} // namespace sovereign
