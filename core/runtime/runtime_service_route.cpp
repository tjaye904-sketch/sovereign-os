#include "runtime_service_route.hpp"

#include <algorithm>
#include <utility>

namespace sovereign
{

RuntimeServiceRouteRegistrationResult
RuntimeServiceRouteRegistry::Register(
    std::string route_name,
    std::string service_name)
{
    RuntimeServiceRouteRegistrationResult result{};

    result.route_name = std::move(route_name);
    result.service_name = std::move(service_name);

    result.size_before = routes_.size();
    result.size_after = routes_.size();

    if (result.route_name.empty())
    {
        result.status =
            RuntimeServiceRouteRegistrationStatus::
                EmptyRouteName;

        last_registration_ = result;
        return result;
    }

    if (result.service_name.empty())
    {
        result.status =
            RuntimeServiceRouteRegistrationStatus::
                EmptyServiceName;

        last_registration_ = result;
        return result;
    }

    if (Contains(result.route_name))
    {
        result.status =
            RuntimeServiceRouteRegistrationStatus::
                DuplicateRoute;

        last_registration_ = result;
        return result;
    }

    result.ordinal = routes_.size();

    routes_.push_back(
        RuntimeServiceRoute{
            .route_name = result.route_name,
            .service_name = result.service_name,
            .state =
                RuntimeServiceRouteState::Enabled,
            .ordinal = result.ordinal});

    ++generation_;

    result.status =
        RuntimeServiceRouteRegistrationStatus::
            Accepted;

    result.size_after = routes_.size();

    last_registration_ = result;
    return result;
}

bool RuntimeServiceRouteRegistry::Unregister(
    const std::string_view route_name)
{
    const auto iterator =
        std::find_if(
            routes_.begin(),
            routes_.end(),
            [route_name](
                const RuntimeServiceRoute& route)
            {
                return
                    route.route_name == route_name;
            });

    if (iterator == routes_.end())
    {
        return false;
    }

    routes_.erase(iterator);

    for (std::size_t index = 0U;
         index < routes_.size();
         ++index)
    {
        routes_[index].ordinal = index;
    }

    ++generation_;
    return true;
}

bool RuntimeServiceRouteRegistry::Enable(
    const std::string_view route_name) noexcept
{
    RuntimeServiceRoute* route = Find(route_name);

    if (route == nullptr ||
        route->state ==
            RuntimeServiceRouteState::Enabled)
    {
        return false;
    }

    route->state =
        RuntimeServiceRouteState::Enabled;

    ++generation_;
    return true;
}

bool RuntimeServiceRouteRegistry::Disable(
    const std::string_view route_name) noexcept
{
    RuntimeServiceRoute* route = Find(route_name);

    if (route == nullptr ||
        route->state ==
            RuntimeServiceRouteState::Disabled)
    {
        return false;
    }

    route->state =
        RuntimeServiceRouteState::Disabled;

    ++generation_;
    return true;
}

RuntimeServiceRoute*
RuntimeServiceRouteRegistry::Find(
    const std::string_view route_name) noexcept
{
    for (auto& route : routes_)
    {
        if (route.route_name == route_name)
        {
            return &route;
        }
    }

    return nullptr;
}

const RuntimeServiceRoute*
RuntimeServiceRouteRegistry::Find(
    const std::string_view route_name) const noexcept
{
    for (const auto& route : routes_)
    {
        if (route.route_name == route_name)
        {
            return &route;
        }
    }

    return nullptr;
}

bool RuntimeServiceRouteRegistry::Contains(
    const std::string_view route_name) const noexcept
{
    return Find(route_name) != nullptr;
}

std::vector<RuntimeServiceRoute>
RuntimeServiceRouteRegistry::Inventory() const
{
    return routes_;
}

const RuntimeServiceRouteRegistrationResult&
RuntimeServiceRouteRegistry::
LastRegistration() const noexcept
{
    return last_registration_;
}

std::uint64_t
RuntimeServiceRouteRegistry::Generation() const noexcept
{
    return generation_;
}

std::size_t
RuntimeServiceRouteRegistry::Size() const noexcept
{
    return routes_.size();
}

bool RuntimeServiceRouteRegistry::Empty() const noexcept
{
    return routes_.empty();
}

} // namespace sovereign
