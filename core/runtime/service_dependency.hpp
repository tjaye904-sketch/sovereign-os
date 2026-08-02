#pragma once

#include "service_registry.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace sovereign
{

enum class ServiceDependencyStatus : std::uint8_t
{
    NotResolved,
    Ready,
    MissingDependency,
    DependencyCycle
};

struct MissingServiceDependency final
{
    std::string service_name{};
    std::string dependency_name{};
};

struct ServiceDependencyResolution final
{
    ServiceDependencyStatus status{
        ServiceDependencyStatus::NotResolved};

    std::vector<Service*> startup_order{};
    std::vector<MissingServiceDependency>
        missing_dependencies{};
    std::vector<std::string> cycle_services{};

    [[nodiscard]]
    bool Ready() const noexcept
    {
        return status ==
               ServiceDependencyStatus::Ready;
    }
};

class ServiceDependencyResolver final
{
public:
    [[nodiscard]]
    ServiceDependencyResolution Resolve(
        ServiceRegistry& registry) const;
};

} // namespace sovereign
