#include "event_bus.hpp"
#include "runtime_message_request.hpp"
#include "runtime_service_capability_negotiator.hpp"
#include "runtime_service_capability_protocol.hpp"
#include "runtime_service_dispatcher.hpp"
#include "runtime_service_endpoint.hpp"
#include "runtime_service_endpoint_protocol.hpp"
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

class CapabilityNegotiationService final
    : public sovereign::Service
{
public:
    CapabilityNegotiationService(
        std::string name,
        std::vector<std::string_view> capabilities)
        :
        name_(std::move(name)),
        capabilities_(std::move(capabilities))
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

private:
    std::string name_;
    std::vector<std::string_view> capabilities_;
};

sovereign::RuntimeMessageRequest MakeRequest(
    const std::string& capability,
    const std::string& preferred,
    const bool require_running)
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
                                            Capability,
                                .target_name =
                                    capability,
                                .operation =
                                    preferred,
                                .payload =
                                    require_running
                                        ? "1"
                                        : "0"}),
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
                CapabilityNegotiationService>(
                    "PrimaryStorage",
                    std::vector<std::string_view>{
                        "storage.read",
                        "storage.write"})));

    REQUIRE(
        registry.Register(
            std::make_unique<
                CapabilityNegotiationService>(
                    "ReplicaStorage",
                    std::vector<std::string_view>{
                        "storage.read"})));

    sovereign::RuntimeServiceCapabilityNegotiator
        negotiator(registry);

    /*
     * Created services may be selected when running
     * state is not required.
     */
    const auto created =
        negotiator.Negotiate(
            sovereign::
                RuntimeServiceCapabilityRequest{
                    .capability = "storage.read",
                    .require_running = false});

    REQUIRE(created.Accepted());
    REQUIRE(
        created.selected_service_name ==
        "PrimaryStorage");
    REQUIRE(created.selected_service_ordinal == 0U);
    REQUIRE(created.candidate_count == 2U);
    REQUIRE(created.negotiation_sequence == 1U);

    /*
     * Running-only negotiation initially fails.
     */
    const auto not_running =
        negotiator.Negotiate(
            sovereign::
                RuntimeServiceCapabilityRequest{
                    .capability = "storage.read",
                    .require_running = true});

    REQUIRE(!not_running.Accepted());

    REQUIRE(
        not_running.status ==
        sovereign::
            RuntimeServiceCapabilityStatus::
                NoEligibleProvider);

    sovereign::EventBus event_bus;
    sovereign::ServiceHost host(
        event_bus,
        registry);

    REQUIRE(host.StartAll());

    /*
     * Preferred provider selection is deterministic.
     */
    const auto preferred =
        negotiator.Negotiate(
            sovereign::
                RuntimeServiceCapabilityRequest{
                    .capability = "storage.read",
                    .preferred_service_name =
                        "ReplicaStorage",
                    .require_running = true});

    REQUIRE(preferred.Accepted());

    REQUIRE(
        preferred.selected_service_name ==
        "ReplicaStorage");

    REQUIRE(preferred.selected_service_ordinal == 1U);
    REQUIRE(preferred.candidate_count == 2U);

    const auto missing =
        negotiator.Negotiate(
            sovereign::
                RuntimeServiceCapabilityRequest{
                    .capability = "missing",
                    .require_running = true});

    REQUIRE(
        missing.status ==
        sovereign::
            RuntimeServiceCapabilityStatus::
                CapabilityNotFound);

    const auto unavailable_preferred =
        negotiator.Negotiate(
            sovereign::
                RuntimeServiceCapabilityRequest{
                    .capability = "storage.read",
                    .preferred_service_name =
                        "MissingStorage",
                    .require_running = true});

    REQUIRE(
        unavailable_preferred.status ==
        sovereign::
            RuntimeServiceCapabilityStatus::
                PreferredProviderUnavailable);

    /*
     * Protocol preserves negotiation requests and
     * results containing delimiters.
     */
    const auto encoded_request =
        sovereign::
            RuntimeServiceCapabilityProtocol::
                EncodeRequest(
                    sovereign::
                        RuntimeServiceCapabilityRequest{
                            .capability =
                                "storage:read\nv1",
                            .preferred_service_name =
                                "Storage:Primary",
                            .require_running = false});

    const auto decoded_request =
        sovereign::
            RuntimeServiceCapabilityProtocol::
                DecodeRequest(encoded_request);

    REQUIRE(decoded_request.has_value());

    REQUIRE(
        decoded_request->capability ==
        "storage:read\nv1");

    REQUIRE(
        decoded_request->preferred_service_name ==
        "Storage:Primary");

    REQUIRE(!decoded_request->require_running);

    const auto encoded_result =
        sovereign::
            RuntimeServiceCapabilityProtocol::
                EncodeResult(preferred);

    const auto decoded_result =
        sovereign::
            RuntimeServiceCapabilityProtocol::
                DecodeResult(encoded_result);

    REQUIRE(decoded_result.has_value());
    REQUIRE(decoded_result->Accepted());

    REQUIRE(
        decoded_result->selected_service_name ==
        "ReplicaStorage");

    /*
     * Negotiation is available through RT-110.
     */
    sovereign::RuntimeServiceInvoker
        invoker(registry);

    sovereign::RuntimeServiceRouteRegistry routes;

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
        dispatcher,
        negotiator);

    REQUIRE(endpoint.Start());

    const auto endpoint_response =
        messaging.Request(
            MakeRequest(
                "storage.read",
                "ReplicaStorage",
                true));

    REQUIRE(endpoint_response.Completed());

    const auto endpoint_result =
        sovereign::
            RuntimeServiceEndpointProtocol::
                DecodeResult(
                    endpoint_response.response_payload);

    REQUIRE(endpoint_result.has_value());
    REQUIRE(endpoint_result->Completed());

    REQUIRE(
        endpoint_result->target ==
        sovereign::
            RuntimeServiceEndpointTarget::
                Capability);

    const auto* negotiated =
        std::get_if<
            sovereign::
                RuntimeServiceCapabilityResult>(
                    &endpoint_result->result);

    REQUIRE(negotiated != nullptr);
    REQUIRE(negotiated->Accepted());

    REQUIRE(
        negotiated->selected_service_name ==
        "ReplicaStorage");

    REQUIRE(endpoint.RequestCount() == 1U);
    REQUIRE(endpoint.ResponseCount() == 1U);
    REQUIRE(endpoint.DirectCount() == 0U);
    REQUIRE(endpoint.RoutedCount() == 0U);
    REQUIRE(endpoint.CapabilityCount() == 1U);
    REQUIRE(endpoint.MalformedCount() == 0U);

    REQUIRE(endpoint.Stop());
    REQUIRE(host.StopAll());

    REQUIRE(negotiator.NegotiationCount() == 6U);
    REQUIRE(negotiator.AcceptedCount() == 3U);

    return 0;
}
