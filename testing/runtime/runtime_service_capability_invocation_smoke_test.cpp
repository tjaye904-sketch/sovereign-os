#include "event_bus.hpp"
#include "runtime_message_request.hpp"
#include "runtime_service_capability_invocation_protocol.hpp"
#include "runtime_service_capability_invoker.hpp"
#include "runtime_service_capability_negotiator.hpp"
#include "runtime_service_dispatcher.hpp"
#include "runtime_service_endpoint.hpp"
#include "runtime_service_endpoint_protocol.hpp"
#include "runtime_service_invocation.hpp"
#include "runtime_service_invoker.hpp"
#include "service_host.hpp"
#include "service_registry.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include "test_require.hpp"

namespace
{

class CapabilityInvocableService final
    : public sovereign::Service,
      public sovereign::RuntimeInvocableService
{
public:
    CapabilityInvocableService(
        std::string name,
        std::vector<std::string_view> capabilities,
        std::string response_prefix)
        :
        name_(std::move(name)),
        capabilities_(std::move(capabilities)),
        response_prefix_(std::move(response_prefix))
    {
    }

    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return name_;
    }

    [[nodiscard]]
    std::vector<std::string_view>
    Capabilities() const override
    {
        return capabilities_;
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
        return operation == "read";
    }

    bool Invoke(
        const std::string_view operation,
        const std::string_view request_payload,
        std::string& response_payload) override
    {
        if (!SupportsOperation(operation))
        {
            return false;
        }

        response_payload =
            response_prefix_ +
            std::string{request_payload};

        return true;
    }

private:
    std::string name_;
    std::vector<std::string_view> capabilities_;
    std::string response_prefix_;
};

sovereign::RuntimeMessageRequest
MakeEndpointRequest(
    const std::string& capability,
    const std::string& operation,
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
        .payload =
            sovereign::
                RuntimeServiceEndpointProtocol::
                    EncodeRequest(
                        sovereign::
                            RuntimeServiceEndpointRequest{
                                .target =
                                    sovereign::
                                        RuntimeServiceEndpointTarget::
                                            CapabilityInvocation,
                                .target_name =
                                    capability,
                                .operation =
                                    operation,
                                .payload =
                                    payload}),
        .security =
            sovereign::EventSecurity::Restricted,
        .maximum_process_cycles = 2U};
}

} // namespace

int main()
{
    sovereign::ServiceRegistry registry;

    REQUIRE(
        registry.Register(
            std::make_unique<
                CapabilityInvocableService>(
                    "PrimaryStorage",
                    std::vector<std::string_view>{
                        "storage.read"},
                    "primary:")));

    REQUIRE(
        registry.Register(
            std::make_unique<
                CapabilityInvocableService>(
                    "ReplicaStorage",
                    std::vector<std::string_view>{
                        "storage.read"},
                    "replica:")));

    sovereign::EventBus event_bus;

    sovereign::ServiceHost host(
        event_bus,
        registry);

    REQUIRE(host.StartAll());

    sovereign::RuntimeServiceInvoker
        direct_invoker(registry);

    sovereign::RuntimeServiceCapabilityNegotiator
        negotiator(registry);

    sovereign::RuntimeServiceCapabilityInvoker
        capability_invoker(
            negotiator,
            direct_invoker);

    /*
     * Default provider selection invokes the first
     * registered running provider.
     */
    const auto default_result =
        capability_invoker.Invoke(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability = "storage.read",
                    .operation = "read",
                    .payload = "alpha"});

    REQUIRE(default_result.Completed());

    REQUIRE(
        default_result.negotiation
            .selected_service_name ==
        "PrimaryStorage");

    REQUIRE(
        default_result.invocation.response_payload ==
        "primary:alpha");

    REQUIRE(
        default_result
            .capability_invocation_sequence == 1U);

    /*
     * A preferred provider is honored.
     */
    const auto preferred_result =
        capability_invoker.Invoke(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability = "storage.read",
                    .preferred_service_name =
                        "ReplicaStorage",
                    .operation = "read",
                    .payload = "beta"});

    REQUIRE(preferred_result.Completed());

    REQUIRE(
        preferred_result.negotiation
            .selected_service_name ==
        "ReplicaStorage");

    REQUIRE(
        preferred_result.invocation.response_payload ==
        "replica:beta");

    /*
     * Missing capabilities fail before direct
     * invocation.
     */
    const std::uint64_t direct_before_missing =
        direct_invoker.InvocationCount();

    const auto missing_capability =
        capability_invoker.Invoke(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability = "storage.missing",
                    .operation = "read",
                    .payload = "ignored"});

    REQUIRE(!missing_capability.Completed());

    REQUIRE(
        missing_capability.status ==
        sovereign::
            RuntimeServiceCapabilityInvocationStatus::
                NegotiationRejected);

    REQUIRE(
        direct_invoker.InvocationCount() ==
        direct_before_missing);

    /*
     * Unsupported operations preserve negotiation
     * success but report invocation rejection.
     */
    const auto rejected_operation =
        capability_invoker.Invoke(
            sovereign::
                RuntimeServiceCapabilityInvocationRequest{
                    .capability = "storage.read",
                    .operation = "write",
                    .payload = "gamma"});

    REQUIRE(!rejected_operation.Completed());
    REQUIRE(rejected_operation.negotiation.Accepted());

    REQUIRE(
        rejected_operation.status ==
        sovereign::
            RuntimeServiceCapabilityInvocationStatus::
                InvocationRejected);

    REQUIRE(
        rejected_operation.invocation.status ==
        sovereign::RuntimeServiceInvocationStatus::
            OperationRejected);

    /*
     * RT-112 request and result protocol round-trip.
     */
    const auto encoded_request =
        sovereign::
            RuntimeServiceCapabilityInvocationProtocol::
                EncodeRequest(
                    sovereign::
                        RuntimeServiceCapabilityInvocationRequest{
                            .capability =
                                "storage:read\nv1",
                            .preferred_service_name =
                                "Replica:Storage",
                            .operation = "read:value",
                            .payload = "payload\nvalue"});

    const auto decoded_request =
        sovereign::
            RuntimeServiceCapabilityInvocationProtocol::
                DecodeRequest(encoded_request);

    REQUIRE(decoded_request.has_value());

    REQUIRE(
        decoded_request->capability ==
        "storage:read\nv1");

    REQUIRE(
        decoded_request->preferred_service_name ==
        "Replica:Storage");

    REQUIRE(
        decoded_request->operation ==
        "read:value");

    REQUIRE(
        decoded_request->payload ==
        "payload\nvalue");

    const auto encoded_result =
        sovereign::
            RuntimeServiceCapabilityInvocationProtocol::
                EncodeResult(preferred_result);

    const auto decoded_result =
        sovereign::
            RuntimeServiceCapabilityInvocationProtocol::
                DecodeResult(encoded_result);

    REQUIRE(decoded_result.has_value());
    REQUIRE(decoded_result->Completed());

    REQUIRE(
        decoded_result->invocation.response_payload ==
        "replica:beta");

    /*
     * RT-112 is available through the unified
     * endpoint as an independent fourth target.
     */
    sovereign::RuntimeServiceRouteRegistry routes;

    sovereign::RuntimeServiceDispatcher dispatcher(
        routes,
        direct_invoker);

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
        direct_invoker,
        dispatcher,
        negotiator,
        capability_invoker);

    REQUIRE(endpoint.Start());

    const auto response =
        messaging.Request(
            MakeEndpointRequest(
                "storage.read",
                "read",
                "delta"));

    REQUIRE(response.Completed());

    const auto endpoint_result =
        sovereign::
            RuntimeServiceEndpointProtocol::
                DecodeResult(
                    response.response_payload);

    REQUIRE(endpoint_result.has_value());
    REQUIRE(endpoint_result->Completed());

    REQUIRE(
        endpoint_result->target ==
        sovereign::
            RuntimeServiceEndpointTarget::
                CapabilityInvocation);

    const auto* capability_result =
        std::get_if<
            sovereign::
                RuntimeServiceCapabilityInvocationResult>(
                    &endpoint_result->result);

    REQUIRE(capability_result != nullptr);
    REQUIRE(capability_result->Completed());

    REQUIRE(
        capability_result
            ->negotiation.selected_service_name ==
        "PrimaryStorage");

    REQUIRE(
        capability_result
            ->invocation.response_payload ==
        "primary:delta");

    REQUIRE(endpoint.RequestCount() == 1U);
    REQUIRE(endpoint.ResponseCount() == 1U);
    REQUIRE(endpoint.DirectCount() == 0U);
    REQUIRE(endpoint.RoutedCount() == 0U);
    REQUIRE(endpoint.CapabilityCount() == 0U);

    REQUIRE(
        endpoint.CapabilityInvocationCount() == 1U);

    REQUIRE(endpoint.MalformedCount() == 0U);

    REQUIRE(endpoint.Stop());
    REQUIRE(host.StopAll());

    REQUIRE(capability_invoker.InvocationCount() == 5U);
    REQUIRE(capability_invoker.CompletedCount() == 3U);

    return 0;
}
