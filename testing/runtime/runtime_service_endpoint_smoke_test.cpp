#include "event_bus.hpp"
#include "runtime_message_request.hpp"
#include "runtime_service_dispatcher.hpp"
#include "runtime_service_endpoint.hpp"
#include "runtime_service_endpoint_protocol.hpp"
#include "runtime_service_invocation.hpp"
#include "runtime_service_invoker.hpp"
#include "runtime_service_message_gateway.hpp"
#include "runtime_service_message_protocol.hpp"
#include "runtime_service_route.hpp"
#include "runtime_service_routed_message_gateway.hpp"
#include "runtime_service_routed_message_protocol.hpp"
#include "service_host.hpp"
#include "service_registry.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include "test_require.hpp"

namespace
{

class UnifiedEndpointService final
    : public sovereign::Service,
      public sovereign::RuntimeInvocableService
{
public:
    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return "UnifiedEndpointService";
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

        if (operation != "echo")
        {
            return false;
        }

        response =
            "unified:" + std::string{payload};

        return true;
    }

    std::size_t invoke_count{0U};
};

sovereign::RuntimeMessageRequest MakeRequest(
    const std::string& payload)
{
    return sovereign::RuntimeMessageRequest{
        .request_topic =
            std::string{
                sovereign::RuntimeServiceEndpoint::
                    RequestTopic},
        .response_topic =
            std::string{
                sovereign::RuntimeServiceEndpoint::
                    ResponseTopic},
        .payload = payload,
        .category =
            sovereign::EventCategory::Runtime,
        .priority =
            sovereign::EventPriority::High,
        .security =
            sovereign::EventSecurity::Restricted,
        .maximum_process_cycles = 2U};
}

} // namespace

int main()
{
    sovereign::EventBus event_bus;
    sovereign::ServiceRegistry registry;

    sovereign::ServiceHost host(
        event_bus,
        registry);

    auto service =
        std::make_unique<UnifiedEndpointService>();

    UnifiedEndpointService* service_pointer =
        service.get();

    REQUIRE(registry.Register(std::move(service)));
    REQUIRE(host.StartAll());

    sovereign::RuntimeServiceInvoker
        invoker(registry);

    sovereign::RuntimeServiceRouteRegistry routes;

    REQUIRE(
        routes.Register(
            "public.unified.echo",
            "UnifiedEndpointService")
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

    sovereign::RuntimeServiceEndpoint endpoint(
        messaging,
        invoker,
        dispatcher);

    const std::size_t subscriptions_before =
        messaging.SubscriptionCount();

    REQUIRE(endpoint.Start());
    REQUIRE(endpoint.Running());
    REQUIRE(!endpoint.Start());

    REQUIRE(
        messaging.SubscriptionCount() ==
        subscriptions_before + 1U);

    /*
     * Direct invocation through the unified endpoint.
     */
    const auto direct =
        messaging.Request(
            MakeRequest(
                sovereign::
                    RuntimeServiceEndpointProtocol::
                        EncodeRequest(
                            sovereign::
                                RuntimeServiceEndpointRequest{
                                    .target =
                                        sovereign::
                                            RuntimeServiceEndpointTarget::
                                                Direct,
                                    .target_name =
                                        "UnifiedEndpointService",
                                    .operation =
                                        "echo",
                                    .payload =
                                        "direct\npayload"})));

    REQUIRE(direct.Completed());

    const auto direct_result =
        sovereign::RuntimeServiceEndpointProtocol::
            DecodeResult(
                direct.response_payload);

    REQUIRE(direct_result.has_value());
    REQUIRE(direct_result->Completed());

    REQUIRE(
        direct_result->target ==
        sovereign::RuntimeServiceEndpointTarget::
            Direct);

    const auto* direct_invocation =
        std::get_if<
            sovereign::RuntimeServiceInvocationResult>(
                &direct_result->result);

    REQUIRE(direct_invocation != nullptr);
    REQUIRE(direct_invocation->Completed());

    REQUIRE(
        direct_invocation->service_name ==
        "UnifiedEndpointService");

    REQUIRE(
        direct_invocation->response_payload ==
        "unified:direct\npayload");

    REQUIRE(
        direct_invocation->invocation_sequence ==
        1U);

    /*
     * Routed invocation through the same endpoint.
     */
    const auto routed =
        messaging.Request(
            MakeRequest(
                sovereign::
                    RuntimeServiceEndpointProtocol::
                        EncodeRequest(
                            sovereign::
                                RuntimeServiceEndpointRequest{
                                    .target =
                                        sovereign::
                                            RuntimeServiceEndpointTarget::
                                                Routed,
                                    .target_name =
                                        "public.unified.echo",
                                    .operation =
                                        "echo",
                                    .payload =
                                        "routed:payload"})));

    REQUIRE(routed.Completed());

    const auto routed_result =
        sovereign::RuntimeServiceEndpointProtocol::
            DecodeResult(
                routed.response_payload);

    REQUIRE(routed_result.has_value());
    REQUIRE(routed_result->Completed());

    REQUIRE(
        routed_result->target ==
        sovereign::RuntimeServiceEndpointTarget::
            Routed);

    const auto* routed_dispatch =
        std::get_if<
            sovereign::RuntimeServiceDispatchResult>(
                &routed_result->result);

    REQUIRE(routed_dispatch != nullptr);
    REQUIRE(routed_dispatch->Completed());

    REQUIRE(
        routed_dispatch->route_name ==
        "public.unified.echo");

    REQUIRE(
        routed_dispatch->service_name ==
        "UnifiedEndpointService");

    REQUIRE(
        routed_dispatch->
            invocation.response_payload ==
        "unified:routed:payload");

    REQUIRE(routed_dispatch->dispatch_sequence == 1U);

    REQUIRE(
        routed_dispatch->
            invocation.invocation_sequence == 2U);

    /*
     * Direct and routed failures retain their native
     * result contracts.
     */
    const auto missing_direct =
        messaging.Request(
            MakeRequest(
                sovereign::
                    RuntimeServiceEndpointProtocol::
                        EncodeRequest(
                            sovereign::
                                RuntimeServiceEndpointRequest{
                                    .target =
                                        sovereign::
                                            RuntimeServiceEndpointTarget::
                                                Direct,
                                    .target_name =
                                        "MissingService",
                                    .operation =
                                        "echo",
                                    .payload =
                                        "payload"})));

    REQUIRE(missing_direct.Completed());

    const auto missing_direct_result =
        sovereign::RuntimeServiceEndpointProtocol::
            DecodeResult(
                missing_direct.response_payload);

    REQUIRE(missing_direct_result.has_value());
    REQUIRE(!missing_direct_result->Completed());

    const auto* missing_invocation =
        std::get_if<
            sovereign::RuntimeServiceInvocationResult>(
                &missing_direct_result->result);

    REQUIRE(missing_invocation != nullptr);

    REQUIRE(
        missing_invocation->status ==
        sovereign::RuntimeServiceInvocationStatus::
            ServiceNotFound);

    const auto missing_route =
        messaging.Request(
            MakeRequest(
                sovereign::
                    RuntimeServiceEndpointProtocol::
                        EncodeRequest(
                            sovereign::
                                RuntimeServiceEndpointRequest{
                                    .target =
                                        sovereign::
                                            RuntimeServiceEndpointTarget::
                                                Routed,
                                    .target_name =
                                        "missing.route",
                                    .operation =
                                        "echo",
                                    .payload =
                                        "payload"})));

    REQUIRE(missing_route.Completed());

    const auto missing_route_result =
        sovereign::RuntimeServiceEndpointProtocol::
            DecodeResult(
                missing_route.response_payload);

    REQUIRE(missing_route_result.has_value());
    REQUIRE(!missing_route_result->Completed());

    const auto* missing_dispatch =
        std::get_if<
            sovereign::RuntimeServiceDispatchResult>(
                &missing_route_result->result);

    REQUIRE(missing_dispatch != nullptr);

    REQUIRE(
        missing_dispatch->status ==
        sovereign::RuntimeServiceDispatchStatus::
            RouteNotFound);

    /*
     * Malformed messages do not reach either backend.
     */
    const auto malformed =
        messaging.Request(
            MakeRequest("invalid-unified-frame"));

    REQUIRE(malformed.Completed());

    const auto malformed_result =
        sovereign::RuntimeServiceEndpointProtocol::
            DecodeResult(
                malformed.response_payload);

    REQUIRE(malformed_result.has_value());
    REQUIRE(!malformed_result->Completed());

    REQUIRE(
        malformed_result->target ==
        sovereign::RuntimeServiceEndpointTarget::
            Direct);

    const auto* malformed_invocation =
        std::get_if<
            sovereign::RuntimeServiceInvocationResult>(
                &malformed_result->result);

    REQUIRE(malformed_invocation != nullptr);

    REQUIRE(
        malformed_invocation->status ==
        sovereign::RuntimeServiceInvocationStatus::
            EmptyServiceName);

    REQUIRE(
        malformed_invocation->diagnostic ==
        "Malformed unified runtime service "
        "endpoint message.");

    REQUIRE(service_pointer->invoke_count == 2U);
    REQUIRE(invoker.InvocationCount() == 3U);
    REQUIRE(dispatcher.DispatchCount() == 2U);

    REQUIRE(endpoint.RequestCount() == 5U);
    REQUIRE(endpoint.ResponseCount() == 5U);
    REQUIRE(endpoint.DirectCount() == 2U);
    REQUIRE(endpoint.RoutedCount() == 2U);
    REQUIRE(endpoint.MalformedCount() == 1U);

    /*
     * RT-107 and RT-109 remain independently usable.
     */
    sovereign::RuntimeServiceMessageGateway
        direct_gateway(messaging, invoker);

    sovereign::RuntimeServiceRoutedMessageGateway
        routed_gateway(messaging, dispatcher);

    REQUIRE(direct_gateway.Start());
    REQUIRE(routed_gateway.Start());

    const auto legacy_direct =
        messaging.Request(
            sovereign::RuntimeMessageRequest{
                .request_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceMessageGateway::
                                RequestTopic},
                .response_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceMessageGateway::
                                ResponseTopic},
                .payload =
                    sovereign::
                        RuntimeServiceMessageProtocol::
                            EncodeRequest(
                                sovereign::
                                    RuntimeServiceMessageRequest{
                                        .service_name =
                                            "UnifiedEndpointService",
                                        .operation =
                                            "echo",
                                        .payload =
                                            "legacy-direct"}),
                .security =
                    sovereign::EventSecurity::
                        Restricted,
                .maximum_process_cycles = 2U});

    REQUIRE(legacy_direct.Completed());

    const auto legacy_routed =
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
                                            "public.unified.echo",
                                        .operation =
                                            "echo",
                                        .payload =
                                            "legacy-routed"}),
                .security =
                    sovereign::EventSecurity::
                        Restricted,
                .maximum_process_cycles = 2U});

    REQUIRE(legacy_routed.Completed());

    REQUIRE(routed_gateway.Stop());
    REQUIRE(direct_gateway.Stop());

    REQUIRE(endpoint.Stop());
    REQUIRE(!endpoint.Running());
    REQUIRE(!endpoint.Stop());

    REQUIRE(
        messaging.SubscriptionCount() ==
        subscriptions_before);

    const auto after_stop =
        messaging.Request(
            MakeRequest(
                sovereign::
                    RuntimeServiceEndpointProtocol::
                        EncodeRequest(
                            sovereign::
                                RuntimeServiceEndpointRequest{
                                    .target =
                                        sovereign::
                                            RuntimeServiceEndpointTarget::
                                                Direct,
                                    .target_name =
                                        "UnifiedEndpointService",
                                    .operation =
                                        "echo",
                                    .payload =
                                        "after-stop"})));

    REQUIRE(!after_stop.Completed());

    REQUIRE(
        after_stop.status ==
        sovereign::RuntimeMessageRequestStatus::
            NoResponse);

    REQUIRE(endpoint.RequestCount() == 5U);

    REQUIRE(host.StopAll());

    return 0;
}
