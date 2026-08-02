#include "service_host.hpp"

namespace sovereign
{

ServiceHost::ServiceHost(
    EventBus& event_bus,
    ServiceRegistry& registry) noexcept
    :
    event_bus_(event_bus),
    registry_(registry)
{
}

bool ServiceHost::StartAll()
{
    last_dependency_resolution_ =
        dependency_resolver_.Resolve(registry_);

    active_order_.clear();

    if (!last_dependency_resolution_.Ready())
    {
        return false;
    }

    for (Service* const service :
         last_dependency_resolution_.startup_order)
    {
        service->SetState(ServiceState::Starting);

        if (!service->Start(event_bus_))
        {
            service->SetState(ServiceState::Failed);
            return false;
        }

        service->SetState(ServiceState::Running);
        active_order_.push_back(service);
    }

    return true;
}

bool ServiceHost::UpdateAll(
    const FrameContext& frame)
{
    for (Service* const service :
         active_order_)
    {
        if (service->State() !=
            ServiceState::Running)
        {
            continue;
        }

        if (!service->Update(frame))
        {
            service->SetState(ServiceState::Failed);
            return false;
        }
    }

    return true;
}

bool ServiceHost::StopAll()
{
    for (auto iterator = active_order_.rbegin();
         iterator != active_order_.rend();
         ++iterator)
    {
        Service& service = **iterator;

        if (service.State() !=
                ServiceState::Running &&
            service.State() !=
                ServiceState::Failed)
        {
            continue;
        }

        service.SetState(ServiceState::Stopping);

        if (!service.Stop())
        {
            service.SetState(ServiceState::Failed);
            return false;
        }

        service.SetState(ServiceState::Stopped);
    }

    active_order_.clear();

    return true;
}

const ServiceDependencyResolution&
ServiceHost::LastDependencyResolution() const noexcept
{
    return last_dependency_resolution_;
}

} // namespace sovereign
