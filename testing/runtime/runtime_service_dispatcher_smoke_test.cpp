#include "event_bus.hpp"
#include "runtime_service_dispatcher.hpp"
#include "runtime_service_invocation.hpp"
#include "runtime_service_invoker.hpp"
#include "runtime_service_route.hpp"
#include "service_host.hpp"
#include "service_registry.hpp"

#include <memory>
#include <string>
#include <string_view>
#include "test_require.hpp"

namespace
{

class RoutedService final
    : public sovereign::Service,
      public sovereign::RuntimeInvocableService
{
public:
    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return "RoutedService";
    }

    bool Start(sovereign::EventBus&) override
    {
        return true;
    }

    bool Update(
        const sovereign::FrameContext&) override
    {
        return true;
    }

    bool Stop() override
    {
        return true;
    }

    [[nodiscard]]
    bool SupportsOperation(
        const std::string_view operation)
        const noexcept override
    {
        return
            operation == "echo" ||
            operation == "reject";
    }

    [[nodiscard]]
    bool Invoke(
        const std::string_view operation,
        const std::string_view payload,
        std::string& response) override
    {
        ++invoke_count;

        if (operation == "echo")
        {
            response =
                "routed:" + std::string{payload};

            return true;
        }

        return false;
    }

    std::size_t invoke_count{0U};
};

} // namespace

int main()
{
    sovereign::EventBus event_bus;
    sovereign::ServiceRegistry services;

    sovereign::ServiceHost host(
        event_bus,
        services);

    auto service =
        std::make_unique<RoutedService>();

    RoutedService* service_pointer =
        service.get();

    REQUIRE(services.Register(std::move(service)));
    REQUIRE(host.StartAll());

    sovereign::RuntimeServiceInvoker
        invoker(services);

    sovereign::RuntimeServiceRouteRegistry
        routes;

    /*
     * Route registration validation.
     */
    const auto empty_route =
        routes.Register("", "RoutedService");

    REQUIRE(!empty_route.Accepted());

    REQUIRE(
        empty_route.status ==
        sovereign::
            RuntimeServiceRouteRegistrationStatus::
                EmptyRouteName);

    const auto empty_service =
        routes.Register("public.echo", "");

    REQUIRE(!empty_service.Accepted());

    REQUIRE(
        empty_service.status ==
        sovereign::
            RuntimeServiceRouteRegistrationStatus::
                EmptyServiceName);

    const auto accepted =
        routes.Register(
            "public.echo",
            "RoutedService");

    REQUIRE(accepted.Accepted());
    REQUIRE(accepted.ordinal == 0U);
    REQUIRE(accepted.size_before == 0U);
    REQUIRE(accepted.size_after == 1U);

    const auto duplicate =
        routes.Register(
            "public.echo",
            "AnotherService");

    REQUIRE(!duplicate.Accepted());

    REQUIRE(
        duplicate.status ==
        sovereign::
            RuntimeServiceRouteRegistrationStatus::
                DuplicateRoute);

    REQUIRE(routes.Size() == 1U);
    REQUIRE(routes.Generation() == 1U);

    sovereign::RuntimeServiceDispatcher
        dispatcher(routes, invoker);

    /*
     * Dispatch validation.
     */
    const auto no_route =
        dispatcher.Dispatch(
            sovereign::
                RuntimeServiceDispatchRequest{
                    .route_name = "",
                    .operation = "echo",
                    .payload = "invalid"});

    REQUIRE(!no_route.Completed());

    REQUIRE(
        no_route.status ==
        sovereign::RuntimeServiceDispatchStatus::
            EmptyRouteName);

    const auto no_operation =
        dispatcher.Dispatch(
            sovereign::
                RuntimeServiceDispatchRequest{
                    .route_name = "public.echo",
                    .operation = "",
                    .payload = "invalid"});

    REQUIRE(!no_operation.Completed());

    REQUIRE(
        no_operation.status ==
        sovereign::RuntimeServiceDispatchStatus::
            EmptyOperation);

    const auto missing_route =
        dispatcher.Dispatch(
            sovereign::
                RuntimeServiceDispatchRequest{
                    .route_name = "missing",
                    .operation = "echo",
                    .payload = "invalid"});

    REQUIRE(!missing_route.Completed());

    REQUIRE(
        missing_route.status ==
        sovereign::RuntimeServiceDispatchStatus::
            RouteNotFound);

    /*
     * Successful routed invocation.
     */
    const auto completed =
        dispatcher.Dispatch(
            sovereign::
                RuntimeServiceDispatchRequest{
                    .route_name = "public.echo",
                    .operation = "echo",
                    .payload = "ping"});

    REQUIRE(completed.Completed());

    REQUIRE(
        completed.service_name ==
        "RoutedService");

    REQUIRE(
        completed.invocation.Completed());

    REQUIRE(
        completed.invocation.response_payload ==
        "routed:ping");

    REQUIRE(completed.dispatch_sequence == 4U);

    REQUIRE(service_pointer->invoke_count == 1U);
    REQUIRE(invoker.InvocationCount() == 1U);
    REQUIRE(dispatcher.DispatchCount() == 4U);
    REQUIRE(dispatcher.CompletedCount() == 1U);

    const auto* route =
        routes.Find("public.echo");

    REQUIRE(route != nullptr);
    REQUIRE(route->dispatch_count == 1U);
    REQUIRE(route->completed_count == 1U);

    /*
     * Invocation failures remain dispatch results.
     */
    const auto rejected =
        dispatcher.Dispatch(
            sovereign::
                RuntimeServiceDispatchRequest{
                    .route_name = "public.echo",
                    .operation = "reject",
                    .payload = "payload"});

    REQUIRE(!rejected.Completed());

    REQUIRE(
        rejected.status ==
        sovereign::RuntimeServiceDispatchStatus::
            InvocationFailed);

    REQUIRE(
        rejected.invocation.status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                OperationRejected);

    REQUIRE(service_pointer->invoke_count == 2U);
    REQUIRE(invoker.InvocationCount() == 2U);
    REQUIRE(dispatcher.DispatchCount() == 5U);
    REQUIRE(dispatcher.CompletedCount() == 1U);

    route = routes.Find("public.echo");

    REQUIRE(route != nullptr);
    REQUIRE(route->dispatch_count == 2U);
    REQUIRE(route->completed_count == 1U);

    /*
     * Route lifecycle.
     */
    REQUIRE(routes.Disable("public.echo"));
    REQUIRE(!routes.Disable("public.echo"));

    const auto disabled =
        dispatcher.Dispatch(
            sovereign::
                RuntimeServiceDispatchRequest{
                    .route_name = "public.echo",
                    .operation = "echo",
                    .payload = "blocked"});

    REQUIRE(!disabled.Completed());

    REQUIRE(
        disabled.status ==
        sovereign::RuntimeServiceDispatchStatus::
            RouteDisabled);

    REQUIRE(service_pointer->invoke_count == 2U);
    REQUIRE(invoker.InvocationCount() == 2U);

    REQUIRE(routes.Enable("public.echo"));
    REQUIRE(!routes.Enable("public.echo"));

    const auto enabled =
        dispatcher.Dispatch(
            sovereign::
                RuntimeServiceDispatchRequest{
                    .route_name = "public.echo",
                    .operation = "echo",
                    .payload = "again"});

    REQUIRE(enabled.Completed());

    REQUIRE(
        enabled.invocation.response_payload ==
        "routed:again");

    REQUIRE(service_pointer->invoke_count == 3U);
    REQUIRE(invoker.InvocationCount() == 3U);

    route = routes.Find("public.echo");

    REQUIRE(route != nullptr);
    REQUIRE(route->dispatch_count == 3U);
    REQUIRE(route->completed_count == 2U);

    /*
     * Deterministic inventory and route removal.
     */
    REQUIRE(
        routes.Register(
            "secondary.echo",
            "RoutedService")
            .Accepted());

    const auto inventory = routes.Inventory();

    REQUIRE(inventory.size() == 2U);
    REQUIRE(inventory[0].route_name == "public.echo");
    REQUIRE(inventory[0].ordinal == 0U);
    REQUIRE(inventory[1].route_name == "secondary.echo");
    REQUIRE(inventory[1].ordinal == 1U);

    REQUIRE(routes.Unregister("public.echo"));
    REQUIRE(!routes.Contains("public.echo"));
    REQUIRE(routes.Size() == 1U);

    const auto compacted = routes.Inventory();

    REQUIRE(compacted.size() == 1U);
    REQUIRE(compacted[0].route_name == "secondary.echo");
    REQUIRE(compacted[0].ordinal == 0U);

    REQUIRE(!routes.Unregister("missing"));

    REQUIRE(
        dispatcher.LastDispatch().Completed());

    REQUIRE(dispatcher.DispatchCount() == 7U);
    REQUIRE(dispatcher.CompletedCount() == 2U);

    REQUIRE(host.StopAll());

    return 0;
}
