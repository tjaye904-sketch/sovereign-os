#include "event_bus.hpp"
#include "runtime_message_request.hpp"
#include "runtime_service_invocation.hpp"
#include "runtime_service_invoker.hpp"
#include "runtime_service_message_gateway.hpp"
#include "runtime_service_message_protocol.hpp"
#include "service_host.hpp"
#include "service_registry.hpp"

#include <memory>
#include <string>
#include <string_view>
#include "test_require.hpp"

namespace
{

class MessageInvocableService final
    : public sovereign::Service,
      public sovereign::RuntimeInvocableService
{
public:
    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return "MessageInvocationService";
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
            "message:" +
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
        std::make_unique<
            MessageInvocableService>();

    MessageInvocableService* service_pointer =
        service.get();

    REQUIRE(registry.Register(std::move(service)));
    REQUIRE(host.StartAll());

    sovereign::RuntimeServiceInvoker
        invoker(registry);

    sovereign::RuntimeId runtime_id =
        sovereign::RuntimeId::Generate();

    sovereign::NodeId node_id =
        sovereign::NodeId::Generate();

    sovereign::RuntimeMessageBus messaging(
        event_bus,
        runtime_id,
        node_id);

    sovereign::RuntimeServiceMessageGateway
        gateway(messaging, invoker);

    const std::size_t subscriptions_before =
        messaging.SubscriptionCount();

    REQUIRE(gateway.Start());
    REQUIRE(gateway.Running());

    REQUIRE(
        messaging.SubscriptionCount() ==
        subscriptions_before + 1U);

    /*
     * Successful invocation through the message bus.
     */
    const auto request_payload =
        sovereign::RuntimeServiceMessageProtocol::
            EncodeRequest(
                sovereign::
                    RuntimeServiceMessageRequest{
                        .service_name =
                            "MessageInvocationService",
                        .operation = "echo",
                        .payload =
                            "ping\nwith:delimiters"});

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
                            RuntimeServiceMessageGateway::
                                RequestTopic},
                .response_topic =
                    std::string{
                        sovereign::
                            RuntimeServiceMessageGateway::
                                ResponseTopic},
                .payload = request_payload,
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

    const auto invocation_result =
        sovereign::RuntimeServiceMessageProtocol::
            DecodeResult(
                completed.response_payload);

    REQUIRE(invocation_result.has_value());
    REQUIRE(invocation_result->Completed());

    REQUIRE(
        invocation_result->service_name ==
        "MessageInvocationService");

    REQUIRE(
        invocation_result->operation == "echo");

    REQUIRE(
        invocation_result->response_payload ==
        "message:ping\nwith:delimiters");

    REQUIRE(
        invocation_result->invocation_sequence ==
        1U);

    REQUIRE(service_pointer->invoke_count == 1U);
    REQUIRE(invoker.InvocationCount() == 1U);
    REQUIRE(invoker.CompletedCount() == 1U);

    REQUIRE(gateway.RequestCount() == 1U);
    REQUIRE(gateway.ResponseCount() == 1U);
    REQUIRE(gateway.MalformedCount() == 0U);

    /*
     * Invocation failures remain valid correlated
     * responses rather than message timeouts.
     */
    const auto missing_service =
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
                                            "MissingService",
                                        .operation =
                                            "echo",
                                        .payload =
                                            "payload"}),
                .security =
                    sovereign::EventSecurity::
                        Restricted,
                .maximum_process_cycles = 2U});

    REQUIRE(missing_service.Completed());

    const auto missing_result =
        sovereign::RuntimeServiceMessageProtocol::
            DecodeResult(
                missing_service.response_payload);

    REQUIRE(missing_result.has_value());

    REQUIRE(
        missing_result->status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                ServiceNotFound);

    REQUIRE(
        missing_result->invocation_sequence ==
        2U);

    REQUIRE(gateway.RequestCount() == 2U);
    REQUIRE(gateway.ResponseCount() == 2U);
    REQUIRE(gateway.MalformedCount() == 0U);

    /*
     * Malformed messages produce deterministic error
     * responses without reaching the invoker.
     */
    const auto malformed =
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
                .payload = "not-a-valid-frame",
                .security =
                    sovereign::EventSecurity::
                        Restricted,
                .maximum_process_cycles = 2U});

    REQUIRE(malformed.Completed());

    const auto malformed_result =
        sovereign::RuntimeServiceMessageProtocol::
            DecodeResult(
                malformed.response_payload);

    REQUIRE(malformed_result.has_value());

    REQUIRE(
        malformed_result->status ==
        sovereign::
            RuntimeServiceInvocationStatus::
                EmptyServiceName);

    REQUIRE(
        malformed_result->diagnostic ==
        "Malformed runtime service invocation "
        "message.");

    REQUIRE(
        malformed_result->invocation_sequence ==
        0U);

    REQUIRE(invoker.InvocationCount() == 2U);
    REQUIRE(gateway.RequestCount() == 3U);
    REQUIRE(gateway.ResponseCount() == 3U);
    REQUIRE(gateway.MalformedCount() == 1U);

    REQUIRE(gateway.Stop());
    REQUIRE(!gateway.Running());

    REQUIRE(
        messaging.SubscriptionCount() ==
        subscriptions_before);

    /*
     * Once detached, no gateway response is produced.
     */
    const auto after_stop =
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
                .payload = request_payload,
                .security =
                    sovereign::EventSecurity::
                        Restricted,
                .maximum_process_cycles = 2U});

    REQUIRE(!after_stop.Completed());

    REQUIRE(
        after_stop.status ==
        sovereign::
            RuntimeMessageRequestStatus::
                NoResponse);

    REQUIRE(invoker.InvocationCount() == 2U);
    REQUIRE(gateway.RequestCount() == 3U);

    REQUIRE(host.StopAll());

    return 0;
}
