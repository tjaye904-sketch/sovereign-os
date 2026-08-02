#include "service_dependency.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sovereign
{

ServiceDependencyResolution
ServiceDependencyResolver::Resolve(
    ServiceRegistry& registry) const
{
    ServiceDependencyResolution result{};

    const auto& services = registry.Services();

    if (services.empty())
    {
        result.status =
            ServiceDependencyStatus::Ready;

        return result;
    }

    std::unordered_map<std::string, std::size_t>
        index_by_name{};

    index_by_name.reserve(services.size());

    for (std::size_t index = 0U;
         index < services.size();
         ++index)
    {
        index_by_name.emplace(
            std::string{services[index]->Name()},
            index);
    }

    std::vector<std::size_t> indegree(
        services.size(),
        0U);

    std::vector<std::vector<std::size_t>>
        dependents(services.size());

    for (std::size_t service_index = 0U;
         service_index < services.size();
         ++service_index)
    {
        const Service& service =
            *services[service_index];

        for (const std::string_view dependency :
             service.Dependencies())
        {
            const auto dependency_iterator =
                index_by_name.find(
                    std::string{dependency});

            if (dependency_iterator ==
                index_by_name.end())
            {
                result.missing_dependencies.push_back(
                    MissingServiceDependency{
                        .service_name =
                            std::string{service.Name()},
                        .dependency_name =
                            std::string{dependency}});
                continue;
            }

            const std::size_t dependency_index =
                dependency_iterator->second;

            dependents[dependency_index].push_back(
                service_index);

            ++indegree[service_index];
        }
    }

    if (!result.missing_dependencies.empty())
    {
        result.status =
            ServiceDependencyStatus::
                MissingDependency;

        return result;
    }

    std::vector<bool> scheduled(
        services.size(),
        false);

    result.startup_order.reserve(
        services.size());

    while (result.startup_order.size() <
           services.size())
    {
        bool made_progress = false;

        for (std::size_t index = 0U;
             index < services.size();
             ++index)
        {
            if (scheduled[index] ||
                indegree[index] != 0U)
            {
                continue;
            }

            scheduled[index] = true;
            made_progress = true;

            result.startup_order.push_back(
                services[index].get());

            for (const std::size_t dependent :
                 dependents[index])
            {
                --indegree[dependent];
            }
        }

        if (!made_progress)
        {
            break;
        }
    }

    if (result.startup_order.size() !=
        services.size())
    {
        result.startup_order.clear();

        for (std::size_t index = 0U;
             index < services.size();
             ++index)
        {
            if (!scheduled[index])
            {
                result.cycle_services.emplace_back(
                    services[index]->Name());
            }
        }

        result.status =
            ServiceDependencyStatus::
                DependencyCycle;

        return result;
    }

    result.status =
        ServiceDependencyStatus::Ready;

    return result;
}

} // namespace sovereign
