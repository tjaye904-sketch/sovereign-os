#include "service_registry.hpp"

#include <algorithm>
#include <utility>

namespace sovereign
{

bool ServiceRegistry::Register(
    std::unique_ptr<Service> service)
{
    return RegisterDetailed(
               std::move(service))
        .Accepted();
}

ServiceRegistrationResult
ServiceRegistry::RegisterDetailed(
    std::unique_ptr<Service> service)
{
    ServiceRegistrationResult result{};
    result.size_before = services_.size();
    result.size_after = services_.size();

    if (!service)
    {
        result.status =
            ServiceRegistrationStatus::NullService;

        last_registration_ = result;
        return result;
    }

    result.service_name =
        std::string{service->Name()};

    if (result.service_name.empty())
    {
        result.status =
            ServiceRegistrationStatus::EmptyName;

        last_registration_ = result;
        return result;
    }

    if (Contains(result.service_name))
    {
        result.status =
            ServiceRegistrationStatus::DuplicateName;

        last_registration_ = result;
        return result;
    }

    result.ordinal = services_.size();

    services_.push_back(std::move(service));

    ++generation_;

    result.status =
        ServiceRegistrationStatus::Accepted;
    result.size_after = services_.size();

    last_registration_ = result;
    return result;
}

std::unique_ptr<Service>
ServiceRegistry::Unregister(
    std::string_view name)
{
    const auto iterator =
        std::find_if(
            services_.begin(),
            services_.end(),
            [name](const auto& service)
            {
                return service->Name() == name;
            });

    if (iterator == services_.end())
    {
        return nullptr;
    }

    if (!CanUnregister((*iterator)->State()))
    {
        return nullptr;
    }

    auto removed_service =
        std::move(*iterator);

    services_.erase(iterator);

    ++generation_;

    return removed_service;
}

Service* ServiceRegistry::Find(
    std::string_view name) noexcept
{
    for (const auto& service : services_)
    {
        if (service->Name() == name)
        {
            return service.get();
        }
    }

    return nullptr;
}

const Service* ServiceRegistry::Find(
    std::string_view name) const noexcept
{
    for (const auto& service : services_)
    {
        if (service->Name() == name)
        {
            return service.get();
        }
    }

    return nullptr;
}

bool ServiceRegistry::Contains(
    std::string_view name) const noexcept
{
    return Find(name) != nullptr;
}

std::vector<ServiceInventoryEntry>
ServiceRegistry::Inventory() const
{
    std::vector<ServiceInventoryEntry> inventory{};
    inventory.reserve(services_.size());

    for (std::size_t index = 0U;
         index < services_.size();
         ++index)
    {
        const Service& service =
            *services_[index];

        inventory.push_back(
            ServiceInventoryEntry{
                .name =
                    std::string{service.Name()},
                .state = service.State(),
                .ordinal = index});
    }

    return inventory;
}

const ServiceRegistrationResult&
ServiceRegistry::LastRegistration() const noexcept
{
    return last_registration_;
}

std::uint64_t
ServiceRegistry::Generation() const noexcept
{
    return generation_;
}

std::size_t ServiceRegistry::Size() const noexcept
{
    return services_.size();
}

bool ServiceRegistry::Empty() const noexcept
{
    return services_.empty();
}

std::vector<std::unique_ptr<Service>>&
ServiceRegistry::Services() noexcept
{
    return services_;
}

const std::vector<std::unique_ptr<Service>>&
ServiceRegistry::Services() const noexcept
{
    return services_;
}

bool ServiceRegistry::CanUnregister(
    ServiceState state) noexcept
{
    switch (state)
    {
        case ServiceState::Created:
        case ServiceState::Stopped:
        case ServiceState::Failed:
            return true;

        case ServiceState::Starting:
        case ServiceState::Running:
        case ServiceState::Stopping:
            return false;
    }

    return false;
}

} // namespace sovereign
