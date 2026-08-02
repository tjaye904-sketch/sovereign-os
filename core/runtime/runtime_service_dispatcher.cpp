#include "runtime_service_dispatcher.hpp"

#include <utility>

namespace sovereign
{

RuntimeServiceDispatcher::RuntimeServiceDispatcher(
    RuntimeServiceRouteRegistry& routes,
    RuntimeServiceInvoker& invoker) noexcept
    :
    routes_(routes),
    invoker_(invoker)
{
}

RuntimeServiceDispatchResult
RuntimeServiceDispatcher::Dispatch(
    RuntimeServiceDispatchRequest request)
{
    RuntimeServiceDispatchResult result{};

    result.route_name =
        std::move(request.route_name);

    result.operation =
        std::move(request.operation);

    result.dispatch_sequence =
        ++dispatch_count_;

    if (result.route_name.empty())
    {
        result.status =
            RuntimeServiceDispatchStatus::
                EmptyRouteName;

        result.diagnostic =
            "Runtime service dispatch requires "
            "a route name.";

        last_dispatch_ = result;
        return result;
    }

    if (result.operation.empty())
    {
        result.status =
            RuntimeServiceDispatchStatus::
                EmptyOperation;

        result.diagnostic =
            "Runtime service dispatch requires "
            "an operation.";

        last_dispatch_ = result;
        return result;
    }

    RuntimeServiceRoute* route =
        routes_.Find(result.route_name);

    if (route == nullptr)
    {
        result.status =
            RuntimeServiceDispatchStatus::
                RouteNotFound;

        result.diagnostic =
            "The requested runtime service route "
            "is not registered.";

        last_dispatch_ = result;
        return result;
    }

    result.service_name = route->service_name;

    if (route->state !=
        RuntimeServiceRouteState::Enabled)
    {
        result.status =
            RuntimeServiceDispatchStatus::
                RouteDisabled;

        result.diagnostic =
            "The requested runtime service route "
            "is disabled.";

        last_dispatch_ = result;
        return result;
    }

    ++route->dispatch_count;

    result.invocation =
        invoker_.Invoke(
            RuntimeServiceInvocationRequest{
                .service_name =
                    result.service_name,
                .operation =
                    result.operation,
                .payload =
                    std::move(request.payload)});

    if (!result.invocation.Completed())
    {
        result.status =
            RuntimeServiceDispatchStatus::
                InvocationFailed;

        result.diagnostic =
            result.invocation.diagnostic;

        last_dispatch_ = result;
        return result;
    }

    ++route->completed_count;
    ++completed_count_;

    result.status =
        RuntimeServiceDispatchStatus::Completed;

    last_dispatch_ = result;
    return result;
}

const RuntimeServiceDispatchResult&
RuntimeServiceDispatcher::LastDispatch() const noexcept
{
    return last_dispatch_;
}

std::uint64_t
RuntimeServiceDispatcher::DispatchCount() const noexcept
{
    return dispatch_count_;
}

std::uint64_t
RuntimeServiceDispatcher::CompletedCount() const noexcept
{
    return completed_count_;
}

} // namespace sovereign
