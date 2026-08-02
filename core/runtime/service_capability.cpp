#include "service_capability.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <unordered_set>
#include <utility>

namespace sovereign
{

namespace
{

using CapabilityMap =
    std::map<
        std::string,
        std::vector<ServiceCapabilityProvider>>;

CapabilityMap BuildCapabilityMap(
    const ServiceRegistry& registry)
{
    CapabilityMap capability_map{};

    const auto& services = registry.Services();

    for (std::size_t ordinal = 0U;
         ordinal < services.size();
         ++ordinal)
    {
        const Service& service = *services[ordinal];

        std::unordered_set<std::string>
            service_capabilities{};

        for (const std::string_view capability :
             service.Capabilities())
        {
            if (capability.empty())
            {
                continue;
            }

            const std::string normalized{
                capability};

            if (!service_capabilities
                     .insert(normalized)
                     .second)
            {
                continue;
            }

            capability_map[normalized].push_back(
                ServiceCapabilityProvider{
                    .service_name =
                        std::string{service.Name()},
                    .service_ordinal = ordinal,
                    .service_state =
                        service.State()});
        }
    }

    return capability_map;
}

} // namespace

ServiceCapabilityDiscovery::
ServiceCapabilityDiscovery(
    const ServiceRegistry& registry) noexcept
    :
    registry_(registry)
{
}

bool ServiceCapabilityDiscovery::HasCapability(
    std::string_view capability) const
{
    if (capability.empty())
    {
        return false;
    }

    return !Providers(capability).empty();
}

std::vector<ServiceCapabilityProvider>
ServiceCapabilityDiscovery::Providers(
    std::string_view capability) const
{
    if (capability.empty())
    {
        return {};
    }

    const CapabilityMap capability_map =
        BuildCapabilityMap(registry_);

    const auto iterator =
        capability_map.find(
            std::string{capability});

    if (iterator == capability_map.end())
    {
        return {};
    }

    return iterator->second;
}

std::vector<ServiceCapabilityEntry>
ServiceCapabilityDiscovery::Inventory() const
{
    const CapabilityMap capability_map =
        BuildCapabilityMap(registry_);

    std::vector<ServiceCapabilityEntry> inventory{};
    inventory.reserve(capability_map.size());

    for (const auto& [capability, providers] :
         capability_map)
    {
        inventory.push_back(
            ServiceCapabilityEntry{
                .capability = capability,
                .providers = providers});
    }

    return inventory;
}

std::size_t
ServiceCapabilityDiscovery::CapabilityCount() const
{
    return BuildCapabilityMap(registry_).size();
}

} // namespace sovereign
