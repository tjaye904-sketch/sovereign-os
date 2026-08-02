#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sovereign
{

enum class RuntimeServiceRouteState : std::uint8_t
{
    Enabled,
    Disabled
};

struct RuntimeServiceRoute final
{
    std::string route_name{};
    std::string service_name{};

    RuntimeServiceRouteState state{
        RuntimeServiceRouteState::Enabled};

    std::size_t ordinal{0U};

    std::uint64_t dispatch_count{0U};
    std::uint64_t completed_count{0U};
};

enum class RuntimeServiceRouteRegistrationStatus
    : std::uint8_t
{
    NoAttempt,
    Accepted,
    EmptyRouteName,
    EmptyServiceName,
    DuplicateRoute
};

struct RuntimeServiceRouteRegistrationResult final
{
    RuntimeServiceRouteRegistrationStatus status{
        RuntimeServiceRouteRegistrationStatus::
            NoAttempt};

    std::string route_name{};
    std::string service_name{};

    std::size_t ordinal{0U};
    std::size_t size_before{0U};
    std::size_t size_after{0U};

    [[nodiscard]]
    bool Accepted() const noexcept
    {
        return
            status ==
            RuntimeServiceRouteRegistrationStatus::
                Accepted;
    }
};

class RuntimeServiceRouteRegistry final
{
public:
    [[nodiscard]]
    RuntimeServiceRouteRegistrationResult Register(
        std::string route_name,
        std::string service_name);

    [[nodiscard]]
    bool Unregister(
        std::string_view route_name);

    [[nodiscard]]
    bool Enable(
        std::string_view route_name) noexcept;

    [[nodiscard]]
    bool Disable(
        std::string_view route_name) noexcept;

    [[nodiscard]]
    RuntimeServiceRoute* Find(
        std::string_view route_name) noexcept;

    [[nodiscard]]
    const RuntimeServiceRoute* Find(
        std::string_view route_name) const noexcept;

    [[nodiscard]]
    bool Contains(
        std::string_view route_name) const noexcept;

    [[nodiscard]]
    std::vector<RuntimeServiceRoute>
    Inventory() const;

    [[nodiscard]]
    const RuntimeServiceRouteRegistrationResult&
    LastRegistration() const noexcept;

    [[nodiscard]]
    std::uint64_t Generation() const noexcept;

    [[nodiscard]]
    std::size_t Size() const noexcept;

    [[nodiscard]]
    bool Empty() const noexcept;

private:
    std::vector<RuntimeServiceRoute> routes_{};

    RuntimeServiceRouteRegistrationResult
        last_registration_{};

    std::uint64_t generation_{0U};
};

} // namespace sovereign
