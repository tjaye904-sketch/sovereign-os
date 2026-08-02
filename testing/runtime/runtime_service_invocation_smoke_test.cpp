#include "event_bus.hpp"
#include "runtime_service_invocation.hpp"
#include "runtime_service_invoker.hpp"
#include "service_host.hpp"
#include "service_registry.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include "test_require.hpp"

namespace
{

class InvocableTestService final
    : public sovereign::Service,
      public sovereign::RuntimeInvocableService
{
public:
    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return "InvocationService";
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
        std::string_view operation)
        const noexcept override
    {
        return
            operation == "echo" ||
            operation == "reject" ||
            operation == "throw";
    }

    [[nodiscard]]
    bool Invoke(
        std::string_view operation,
        std::string_view request_payload,
        std::string& response_payload) override
    {
        ++invoke_count;

        if (operation == "echo")
        {
            response_payload =
                "echo:" +
                std::string{request_payload};

            return true;
        }

        if (operation == "reject")
        {
            return false;
        }

        if (operation == "throw")
        {
            throw std::runtime_error{
                "invocation failure"};
        }

        return false;
    }

    std::size_t invoke_count{0U};
};

class PassiveTestService final
    : public sovereign::Service
{
public:
    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return "PassiveService";
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
};

} // namespace

int main()
{
    sovereign::EventBus event_bus;
    sovereign::ServiceRegistry registry;
    sovereign::ServiceHost host(
        event_bus,
        registry);

    auto invocable_service =
        std::make_unique<
            InvocableTestService>();

    InvocableTestService* service_pointer =
        invocable_service.get();

    REQUIRE(
        registry.Register(
            std::move(invocable_service)));

    REQUIRE(
        registry.Register(
            std::make_unique<
                PassiveTestService>()));

    sovereign::RuntimeServiceInvoker
        invoker(registry);

    /*
     * Request validation.
     */
    const auto empty_service =
        invoker.Invoke(
            sovereign::
                RuntimeServiceInvocationRequest{
                    .service_name = "",
                    .operation = "echo",
                    .payload = "payload"});

    REQUIRE(!empty_service.Completed());
    REQUIRE(
        empty_service.status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                EmptyServiceName);

    const auto empty_operation =
        invoker.Invoke(
            sovereign::
                RuntimeServiceInvocationRequest{
                    .service_name =
                        "InvocationService",
                    .operation = "",
                    .payload = "payload"});

    REQUIRE(
        empty_operation.status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                EmptyOperation);

    const auto missing_service =
        invoker.Invoke(
            sovereign::
                RuntimeServiceInvocationRequest{
                    .service_name =
                        "MissingService",
                    .operation = "echo",
                    .payload = "payload"});

    REQUIRE(
        missing_service.status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                ServiceNotFound);

    /*
     * Registered but not running.
     */
    const auto stopped_service =
        invoker.Invoke(
            sovereign::
                RuntimeServiceInvocationRequest{
                    .service_name =
                        "InvocationService",
                    .operation = "echo",
                    .payload = "payload"});

    REQUIRE(
        stopped_service.status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                ServiceNotRunning);

    REQUIRE(host.StartAll());

    /*
     * Service does not expose invocation.
     */
    const auto passive_service =
        invoker.Invoke(
            sovereign::
                RuntimeServiceInvocationRequest{
                    .service_name =
                        "PassiveService",
                    .operation = "echo",
                    .payload = "payload"});

    REQUIRE(
        passive_service.status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                ServiceNotInvocable);

    /*
     * Unsupported operation is rejected before the
     * service handler executes.
     */
    const auto unsupported_operation =
        invoker.Invoke(
            sovereign::
                RuntimeServiceInvocationRequest{
                    .service_name =
                        "InvocationService",
                    .operation = "missing",
                    .payload = "payload"});

    REQUIRE(
        unsupported_operation.status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                OperationRejected);

    REQUIRE(service_pointer->invoke_count == 0U);

    /*
     * Successful invocation.
     */
    const auto completed =
        invoker.Invoke(
            sovereign::
                RuntimeServiceInvocationRequest{
                    .service_name =
                        "InvocationService",
                    .operation = "echo",
                    .payload = "ping"});

    REQUIRE(completed.Completed());
    REQUIRE(
        completed.response_payload ==
        "echo:ping");
    REQUIRE(completed.invocation_sequence == 7U);
    REQUIRE(service_pointer->invoke_count == 1U);

    /*
     * Handler-level rejection.
     */
    const auto rejected =
        invoker.Invoke(
            sovereign::
                RuntimeServiceInvocationRequest{
                    .service_name =
                        "InvocationService",
                    .operation = "reject",
                    .payload = "payload"});

    REQUIRE(
        rejected.status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                OperationRejected);

    REQUIRE(service_pointer->invoke_count == 2U);

    /*
     * Handler exceptions remain contained.
     */
    const auto exception_result =
        invoker.Invoke(
            sovereign::
                RuntimeServiceInvocationRequest{
                    .service_name =
                        "InvocationService",
                    .operation = "throw",
                    .payload = "payload"});

    REQUIRE(
        exception_result.status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                HandlerException);

    REQUIRE(
        exception_result.diagnostic ==
        "invocation failure");

    REQUIRE(service_pointer->invoke_count == 3U);

    REQUIRE(invoker.InvocationCount() == 9U);
    REQUIRE(invoker.CompletedCount() == 1U);

    REQUIRE(
        invoker.LastInvocation().status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                HandlerException);

    REQUIRE(host.StopAll());

    const auto after_stop =
        invoker.Invoke(
            sovereign::
                RuntimeServiceInvocationRequest{
                    .service_name =
                        "InvocationService",
                    .operation = "echo",
                    .payload = "payload"});

    REQUIRE(
        after_stop.status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                ServiceNotRunning);

    REQUIRE(invoker.InvocationCount() == 10U);
    REQUIRE(invoker.CompletedCount() == 1U);

    return 0;
}
