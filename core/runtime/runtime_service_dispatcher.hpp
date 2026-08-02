#pragma once

#include "runtime_service_invocation.hpp"
#include "runtime_service_invoker.hpp"
#include "runtime_service_route.hpp"

#include <cstdint>
#include <string>

namespace sovereign
{

struct RuntimeServiceDispatchRequest final
{
    std::string route_name{};
    std::string operation{};
    std::string payload{};
};

enum class RuntimeServiceDispatchStatus : std::uint8_t
{
    NoAttempt,
    Completed,
    EmptyRouteName,
    EmptyOperation,
    RouteNotFound,
    RouteDisabled,
    InvocationFailed
};

struct RuntimeServiceDispatchResult final
{
    RuntimeServiceDispatchStatus status{
        RuntimeServiceDispatchStatus::NoAttempt};

    std::string route_name{};
    std::string service_name{};
    std::string operation{};

    RuntimeServiceInvocationResult
        invocation{};

    std::string diagnostic{};

    std::uint64_t dispatch_sequence{0U};

    [[nodiscard]]
    bool Completed() const noexcept
    {
        return
            status ==
            RuntimeServiceDispatchStatus::Completed;
    }
};

class RuntimeServiceDispatcher final
{
public:
    RuntimeServiceDispatcher(
        RuntimeServiceRouteRegistry& routes,
        RuntimeServiceInvoker& invoker) noexcept;

    [[nodiscard]]
    RuntimeServiceDispatchResult Dispatch(
        RuntimeServiceDispatchRequest request);

    [[nodiscard]]
    const RuntimeServiceDispatchResult&
    LastDispatch() const noexcept;

    [[nodiscard]]
    std::uint64_t DispatchCount() const noexcept;

    [[nodiscard]]
    std::uint64_t CompletedCount() const noexcept;

private:
    RuntimeServiceRouteRegistry& routes_;
    RuntimeServiceInvoker& invoker_;

    RuntimeServiceDispatchResult last_dispatch_{};

    std::uint64_t dispatch_count_{0U};
    std::uint64_t completed_count_{0U};
};

} // namespace sovereign
