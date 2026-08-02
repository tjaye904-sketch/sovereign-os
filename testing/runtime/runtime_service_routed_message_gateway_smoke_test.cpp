#include "event_bus.hpp"
#include "runtime_message_request.hpp"
#include "runtime_service_dispatcher.hpp"
#include "runtime_service_invocation.hpp"
#include "runtime_service_invoker.hpp"
#include "runtime_service_route.hpp"
#include "runtime_service_routed_message_gateway.hpp"
#include "runtime_service_routed_message_protocol.hpp"
#include "service_host.hpp"
#include "service_registry.hpp"

#include <memory>
#include <string>
#include <string_view>
#include "test_require.hpp"

namespace
{

class RoutedMessageService final
    : public sovereign::Service,
      public sovereign::RuntimeInvocableService
{
public:
    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return "RoutedMessageService";
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
        return operation == "echo";
    }

    [[nodiscard]]
    bool Invoke(
        const std::string_view operation,
        const std::string_view request_payload,
        std::string& response_payload) override
    {
        ++invoke_count;

        if (operation != "echo")
        {
            return false;
        }

        response_payload =
            "routed-message:" +
            std::string{request_payload};

        return true;
    }

    std::size_t invoke_count{0U};
};

} // namespace

int main()
{
    sovereign::EventBus event_bus;
    sovereign::ServiceRegistry registry;

    sovereign::ServiceHost host(
        event_bus,
        registry);

    auto service =
        std::make_unique<RoutedMessageService>();

    RoutedMessageService* service_pointer =
        service.get();

    REQUIRE(registry.Register(std::move(service)));
    REQUIRE(host.StartAll());

    sovereign::RuntimeServiceInvoker
        invoker(registry);

    sovereign::RuntimeServiceRouteRegistry
        routes;

    REQUIRE(
        routes.Register(
            "public.message.echo",
            "RoutedMessageService")
            .Accepted());

    sovereign::RuntimeServiceDispatcher
        dispatcher(routes, invoker);

    const sovereign::RuntimeId runtime_id =
        sovereign::RuntimeId::Generate();

    const sovereign::NodeId node_id =
        sovereign::NodeId::Generate();

    sovereign::RuntimeMessageBus messaging(
        event_bus,
        runtime_id,
        node_id);

    sovereign::RuntimeServiceRoutedMessageGateway
        gateway(messaging, dispatcher);

    const std::size_t subscriptions_before =
        messaging.SubscriptionCount();

    REQUIRE(gateway.Start());
    REQUIRE(gateway.Running());

    REQUIRE(
        messaging.SubscriptionCount() ==
        subscriptions_before + 1U);

    /*
     * Successful routed invocation through messaging.
     */
    const sovereign::TransactionId transaction_id =
        sovereign::TransactionId::Generate();

    const sovereign::DecisionId decision_id =
        sovereign::DecisionId::Generate();

    const auto completed =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceRoutedMessageGateway::
                                RequestTopic},
                .response_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceRoutedMessageGateway::
                                ResponseTopic},
                .payload =
                    sovereign::
                        RuntimeServiceRoutedMessageProtocol::
                            EncodeRequest(
                                sovereign::
                                    RuntimeServiceRoutedMessageRequest{
                                        .route_name =
                                            "public.message.echo",
                                        .operation =
                                            "echo",
                                        .payload =
                                            "ping\nwith:delimiters"}),
                .category =
                    sovereign::EventCategory::Runtime,
                .priority =
                    sovereign::EventPriority::High,
                .security =
                    sovereign::EventSecurity::
                        Restricted,
                .transaction_id = transaction_id,
                .decision_id = decision_id,
                .maximum_process_cycles = 2U});

    REQUIRE(completed.Completed());

    const auto completed_result =
        sovereign::
            RuntimeServiceRoutedMessageProtocol::
                DecodeResult(
                    completed.response_payload);

    REQUIRE(completed_result.has_value());
    REQUIRE(completed_result->Completed());

    REQUIRE(
        completed_result->route_name ==
        "public.message.echo");

    REQUIRE(
        completed_result->service_name ==
        "RoutedMessageService");

    REQUIRE(
        completed_result->operation == "echo");

    REQUIRE(
        completed_result->
            invocation.response_payload ==
        "routed-message:ping\nwith:delimiters");

    REQUIRE(
        completed_result->dispatch_sequence == 1U);

    REQUIRE(
        completed_result->
            invocation.invocation_sequence == 1U);

    REQUIRE(service_pointer->invoke_count == 1U);
    REQUIRE(dispatcher.DispatchCount() == 1U);
    REQUIRE(dispatcher.CompletedCount() == 1U);
    REQUIRE(invoker.InvocationCount() == 1U);
    REQUIRE(invoker.CompletedCount() == 1U);

    REQUIRE(gateway.RequestCount() == 1U);
    REQUIRE(gateway.ResponseCount() == 1U);
    REQUIRE(gateway.MalformedCount() == 0U);

    /*
     * Missing routes produce correlated responses.
     */
    const auto missing =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceRoutedMessageGateway::
                                RequestTopic},
                .response_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceRoutedMessageGateway::
                                ResponseTopic},
                .payload =
                    sovereign::
                        RuntimeServiceRoutedMessageProtocol::
                            EncodeRequest(
                                sovereign::
                                    RuntimeServiceRoutedMessageRequest{
                                        .route_name =
                                            "missing.route",
                                        .operation =
                                            "echo",
                                        .payload =
                                            "payload"}),
                .security =
                    sovereign::EventSecurity::
                        Restricted,
                .maximum_process_cycles = 2U});

    REQUIRE(missing.Completed());

    const auto missing_result =
        sovereign::
            RuntimeServiceRoutedMessageProtocol::
                DecodeResult(
                    missing.response_payload);

    REQUIRE(missing_result.has_value());

    REQUIRE(
        missing_result->status ==
        sovereign::RuntimeServiceDispatchStatus::
            RouteNotFound);

    REQUIRE(
        missing_result->dispatch_sequence == 2U);

    REQUIRE(service_pointer->invoke_count == 1U);
    REQUIRE(invoker.InvocationCount() == 1U);
    REQUIRE(dispatcher.DispatchCount() == 2U);

    /*
     * Disabled routes remain valid responses.
     */
    REQUIRE(routes.Disable("public.message.echo"));

    const auto disabled =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceRoutedMessageGateway::
                                RequestTopic},
                .response_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceRoutedMessageGateway::
                                ResponseTopic},
                .payload =
                    sovereign::
                        RuntimeServiceRoutedMessageProtocol::
                            EncodeRequest(
                                sovereign::
                                    RuntimeServiceRoutedMessageRequest{
                                        .route_name =
                                            "public.message.echo",
                                        .operation =
                                            "echo",
                                        .payload =
                                            "blocked"}),
                .security =
                    sovereign::EventSecurity::
                        Restricted,
                .maximum_process_cycles = 2U});

    REQUIRE(disabled.Completed());

    const auto disabled_result =
        sovereign::
            RuntimeServiceRoutedMessageProtocol::
                DecodeResult(
                    disabled.response_payload);

    REQUIRE(disabled_result.has_value());

    REQUIRE(
        disabled_result->status ==
        sovereign::RuntimeServiceDispatchStatus::
            RouteDisabled);

    REQUIRE(
        disabled_result->dispatch_sequence == 3U);

    REQUIRE(service_pointer->invoke_count == 1U);
    REQUIRE(invoker.InvocationCount() == 1U);

    /*
     * Malformed input does not reach the dispatcher.
     */
    const auto malformed =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceRoutedMessageGateway::
                                RequestTopic},
                .response_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceRoutedMessageGateway::
                                ResponseTopic},
                .payload = "invalid-frame",
                .security =
                    sovereign::EventSecurity::
                        Restricted,
                .maximum_process_cycles = 2U});

    REQUIRE(malformed.Completed());

    const auto malformed_result =
        sovereign::
            RuntimeServiceRoutedMessageProtocol::
                DecodeResult(
                    malformed.response_payload);

    REQUIRE(malformed_result.has_value());

    REQUIRE(
        malformed_result->status ==
        sovereign::RuntimeServiceDispatchStatus::
            EmptyRouteName);

    REQUIRE(
        malformed_result->diagnostic ==
        "Malformed routed runtime service "
        "dispatch message.");

    REQUIRE(
        malformed_result->dispatch_sequence == 0U);

    REQUIRE(dispatcher.DispatchCount() == 3U);
    REQUIRE(gateway.RequestCount() == 4U);
    REQUIRE(gateway.ResponseCount() == 4U);
    REQUIRE(gateway.MalformedCount() == 1U);

    REQUIRE(gateway.Stop());
    REQUIRE(!gateway.Running());

    REQUIRE(
        messaging.SubscriptionCount() ==
        subscriptions_before);

    /*
     * Detached gateway produces no response.
     */
    const auto after_stop =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceRoutedMessageGateway::
                                RequestTopic},
                .response_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceRoutedMessageGateway::
                                ResponseTopic},
                .payload =
                    sovereign::
                        RuntimeServiceRoutedMessageProtocol::
                            EncodeRequest(
                                sovereign::
                                    RuntimeServiceRoutedMessageRequest{
                                        .route_name =
                                            "public.message.echo",
                                        .operation =
                                            "echo",
                                        .payload =
                                            "after-stop"}),
                .security =
                    sovereign::EventSecurity::
                        Restricted,
                .maximum_process_cycles = 2U});

    REQUIRE(!after_stop.Completed());

    REQUIRE(
        after_stop.status ==
        sovereign::RuntimeMessageRequestStatus::
            NoResponse);

    REQUIRE(dispatcher.DispatchCount() == 3U);
    REQUIRE(gateway.RequestCount() == 4U);

    REQUIRE(host.StopAll());

    return 0;
}
