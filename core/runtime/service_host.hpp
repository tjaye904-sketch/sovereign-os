#pragma once

#include "event_bus.hpp"
#include "frame_context.hpp"
#include "service_dependency.hpp"
#include "service_registry.hpp"

#include <vector>

namespace sovereign
{

class ServiceHost final
{
public:
    ServiceHost(
        EventBus& event_bus,
        ServiceRegistry& registry) noexcept;

    bool StartAll();

    bool UpdateAll(
        const FrameContext& frame);

    bool StopAll();

    [[nodiscard]]
    const ServiceDependencyResolution&
    LastDependencyResolution() const noexcept;

private:
    EventBus& event_bus_;
    ServiceRegistry& registry_;

    ServiceDependencyResolver
        dependency_resolver_{};

    ServiceDependencyResolution
        last_dependency_resolution_{};

    std::vector<Service*> active_order_{};
};

} // namespace sovereign
