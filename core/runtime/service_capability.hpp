#pragma once

#include "service_registry.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace sovereign
{

struct ServiceCapabilityProvider final
{
    std::string service_name{};
    std::size_t service_ordinal{0U};
    ServiceState service_state{ServiceState::Created};
};

struct ServiceCapabilityEntry final
{
    std::string capability{};
    std::vector<ServiceCapabilityProvider> providers{};
};

class ServiceCapabilityDiscovery final
{
public:
    explicit ServiceCapabilityDiscovery(
        const ServiceRegistry& registry) noexcept;

    [[nodiscard]]
    bool HasCapability(
        std::string_view capability) const;

    [[nodiscard]]
    std::vector<ServiceCapabilityProvider>
    Providers(
        std::string_view capability) const;

    [[nodiscard]]
    std::vector<ServiceCapabilityEntry>
    Inventory() const;

    [[nodiscard]]
    std::size_t CapabilityCount() const;

private:
    const ServiceRegistry& registry_;
};

} // namespace sovereign
