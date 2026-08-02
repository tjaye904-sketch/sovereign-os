#pragma once

#include "service.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace sovereign
{

enum class ServiceRegistrationStatus : std::uint8_t
{
    NoAttempt,
    Accepted,
    NullService,
    EmptyName,
    DuplicateName
};

struct ServiceRegistrationResult final
{
    ServiceRegistrationStatus status{
        ServiceRegistrationStatus::NoAttempt};

    std::string service_name{};

    std::size_t ordinal{0U};
    std::size_t size_before{0U};
    std::size_t size_after{0U};

    [[nodiscard]]
    bool Accepted() const noexcept
    {
        return status ==
               ServiceRegistrationStatus::Accepted;
    }
};

struct ServiceInventoryEntry final
{
    std::string name{};
    ServiceState state{ServiceState::Created};
    std::size_t ordinal{0U};
};

class ServiceRegistry final
{
public:
    bool Register(std::unique_ptr<Service> service);

    [[nodiscard]]
    ServiceRegistrationResult RegisterDetailed(
        std::unique_ptr<Service> service);

    [[nodiscard]]
    std::unique_ptr<Service> Unregister(
        std::string_view name);

    [[nodiscard]]
    Service* Find(std::string_view name) noexcept;

    [[nodiscard]]
    const Service* Find(
        std::string_view name) const noexcept;

    [[nodiscard]]
    bool Contains(std::string_view name) const noexcept;

    [[nodiscard]]
    std::vector<ServiceInventoryEntry>
    Inventory() const;

    [[nodiscard]]
    const ServiceRegistrationResult&
    LastRegistration() const noexcept;

    [[nodiscard]]
    std::uint64_t Generation() const noexcept;

    [[nodiscard]]
    std::size_t Size() const noexcept;

    [[nodiscard]]
    bool Empty() const noexcept;

    std::vector<std::unique_ptr<Service>>&
    Services() noexcept;

    const std::vector<std::unique_ptr<Service>>&
    Services() const noexcept;

private:
    [[nodiscard]]
    static bool CanUnregister(
        ServiceState state) noexcept;

    std::vector<std::unique_ptr<Service>> services_{};

    ServiceRegistrationResult
        last_registration_{};

    std::uint64_t generation_{0U};
};

} // namespace sovereign
