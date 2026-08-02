#include "event_bus.hpp"
#include "runtime_message_request.hpp"
#include "runtime_service_capability_negotiator.hpp"
#include "runtime_service_capability_provider_selection_policy.hpp"
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
                        "storage.read"})));

    REQUIRE(
        registry.Register(
            std::make_unique<
                CapabilityNegotiationService>(
                    "ReplicaStorage",
                    std::vector<std::string_view>{
                        "storage.read"})));

    REQUIRE(
        registry.Register(
            std::make_unique<
                CapabilityNegotiationService>(
                    "ArchiveStorage",
                    std::vector<std::string_view>{
                        "storage.read"})));

    sovereign::EventBus event_bus;
    sovereign::ServiceHost host(
        event_bus,
        registry);

    REQUIRE(host.StartAll());

    /*
     * Legacy construction preserves registry-ordinal
     * provider selection.
     */
    sovereign::RuntimeServiceCapabilityNegotiator
        legacy_negotiator(registry);

    const auto legacy =
        legacy_negotiator.Negotiate(
            sovereign::
                RuntimeServiceCapabilityRequest{
                    .capability = "storage.read",
                    .require_running = true});

    REQUIRE(legacy.Accepted());
    REQUIRE(
        legacy.selected_service_name ==
        "PrimaryStorage");
    REQUIRE(legacy.selected_service_ordinal == 0U);

    sovereign::
        RuntimeServiceCapabilityProviderSelectionPolicy
            selection_policy;

    REQUIRE(
        selection_policy.SetProviderRank(
            "storage.read",
            "ArchiveStorage",
            20U));

    REQUIRE(
        selection_policy.SetProviderRank(
            "storage.read",
            "ReplicaStorage",
            10U));

    REQUIRE(selection_policy.ProviderRankCount() == 2U);

    sovereign::RuntimeServiceCapabilityNegotiator
        ranked_negotiator(
            registry,
            selection_policy);

    /*
     * Lowest configured rank wins ahead of registry
     * ordinal.
     */
    const auto ranked =
        ranked_negotiator.Negotiate(
            sovereign::
                RuntimeServiceCapabilityRequest{
                    .capability = "storage.read",
                    .require_running = true});

    REQUIRE(ranked.Accepted());

    REQUIRE(
        ranked.selected_service_name ==
        "ReplicaStorage");

    REQUIRE(ranked.selected_service_ordinal == 1U);

    /*
     * An available explicit preference supersedes
     * configured ranking.
     */
    const auto preferred =
        ranked_negotiator.Negotiate(
            sovereign::
                RuntimeServiceCapabilityRequest{
                    .capability = "storage.read",
                    .preferred_service_name =
                        "ArchiveStorage",
                    .require_running = true});

    REQUIRE(preferred.Accepted());

    REQUIRE(
        preferred.selected_service_name ==
        "ArchiveStorage");

    /*
     * Strict preferred-provider behavior remains the
     * policy default.
     */
    const auto strict_missing =
        ranked_negotiator.Negotiate(
            sovereign::
                RuntimeServiceCapabilityRequest{
                    .capability = "storage.read",
                    .preferred_service_name =
                        "MissingStorage",
                    .require_running = true});

    REQUIRE(!strict_missing.Accepted());

    REQUIRE(
        strict_missing.status ==
        sovereign::
            RuntimeServiceCapabilityStatus::
                PreferredProviderUnavailable);

    /*
     * Explicit fallback permits ranked selection when
     * the preferred provider is unavailable.
     */
    selection_policy
        .SetPreferredProviderFallbackEnabled(true);

    const auto fallback =
        ranked_negotiator.Negotiate(
            sovereign::
                RuntimeServiceCapabilityRequest{
                    .capability = "storage.read",
                    .preferred_service_name =
                        "MissingStorage",
                    .require_running = true});

    REQUIRE(fallback.Accepted());

    REQUIRE(
        fallback.selected_service_name ==
        "ReplicaStorage");

    /*
     * Updating a rank changes deterministic selection
     * without rebuilding the negotiator.
     */
    REQUIRE(
        selection_policy.SetProviderRank(
            "storage.read",
            "ArchiveStorage",
            5U));

    const auto updated =
        ranked_negotiator.Negotiate(
            sovereign::
                RuntimeServiceCapabilityRequest{
                    .capability = "storage.read",
                    .require_running = true});

    REQUIRE(updated.Accepted());

    REQUIRE(
        updated.selected_service_name ==
        "ArchiveStorage");

    REQUIRE(
        selection_policy.SelectionCount() == 5U);

    REQUIRE(
        selection_policy.RankedSelectionCount() ==
        3U);

    REQUIRE(
        selection_policy.PreferredSelectionCount() ==
        1U);

    REQUIRE(
        selection_policy.PreferredFallbackCount() ==
        1U);

    REQUIRE(
        selection_policy.RemoveProviderRank(
            "storage.read",
            "ArchiveStorage"));

    REQUIRE(selection_policy.ProviderRankCount() == 1U);

    return 0;
}
