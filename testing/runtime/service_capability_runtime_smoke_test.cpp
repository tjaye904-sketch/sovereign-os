#include "event_bus.hpp"
#include "service_capability.hpp"
#include "service_host.hpp"
#include "service_registry.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include "test_require.hpp"

namespace
{

class CapabilityTestService final
    : public sovereign::Service
{
public:
    CapabilityTestService(
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

} // namespace

int main()
{
    sovereign::ServiceRegistry registry;

    REQUIRE(registry.Register(
        std::make_unique<CapabilityTestService>(
            "StoragePrimary",
            std::vector<std::string_view>{
                "storage.read",
                "storage.write",
                "storage.read",
                ""})));

    REQUIRE(registry.Register(
        std::make_unique<CapabilityTestService>(
            "StorageReplica",
            std::vector<std::string_view>{
                "storage.read",
                "storage.replication"})));

    REQUIRE(registry.Register(
        std::make_unique<CapabilityTestService>(
            "Messaging",
            std::vector<std::string_view>{
                "messaging.publish",
                "messaging.subscribe"})));

    REQUIRE(registry.Register(
        std::make_unique<CapabilityTestService>(
            "Passive",
            std::vector<std::string_view>{})));

    sovereign::ServiceCapabilityDiscovery
        discovery(registry);

    REQUIRE(discovery.CapabilityCount() == 5U);

    REQUIRE(
        discovery.HasCapability("storage.read"));
    REQUIRE(
        discovery.HasCapability("storage.write"));
    REQUIRE(
        discovery.HasCapability(
            "storage.replication"));
    REQUIRE(
        discovery.HasCapability(
            "messaging.publish"));
    REQUIRE(
        discovery.HasCapability(
            "messaging.subscribe"));

    REQUIRE(
        !discovery.HasCapability(""));
    REQUIRE(
        !discovery.HasCapability("missing"));

    const auto read_providers =
        discovery.Providers("storage.read");

    REQUIRE(read_providers.size() == 2U);

    REQUIRE(
        read_providers[0U].service_name ==
        "StoragePrimary");
    REQUIRE(
        read_providers[0U].service_ordinal == 0U);
    REQUIRE(
        read_providers[0U].service_state ==
        sovereign::ServiceState::Created);

    REQUIRE(
        read_providers[1U].service_name ==
        "StorageReplica");
    REQUIRE(
        read_providers[1U].service_ordinal == 1U);

    const auto write_providers =
        discovery.Providers("storage.write");

    REQUIRE(write_providers.size() == 1U);
    REQUIRE(
        write_providers[0U].service_name ==
        "StoragePrimary");

    REQUIRE(
        discovery.Providers("missing").empty());
    REQUIRE(
        discovery.Providers("").empty());

    const auto initial_inventory =
        discovery.Inventory();

    REQUIRE(initial_inventory.size() == 5U);

    REQUIRE(
        initial_inventory[0U].capability ==
        "messaging.publish");
    REQUIRE(
        initial_inventory[1U].capability ==
        "messaging.subscribe");
    REQUIRE(
        initial_inventory[2U].capability ==
        "storage.read");
    REQUIRE(
        initial_inventory[3U].capability ==
        "storage.replication");
    REQUIRE(
        initial_inventory[4U].capability ==
        "storage.write");

    REQUIRE(
        initial_inventory[2U].providers.size() ==
        2U);

    sovereign::EventBus event_bus;
    sovereign::ServiceHost host(
        event_bus,
        registry);

    REQUIRE(host.StartAll());

    const auto running_read_providers =
        discovery.Providers("storage.read");

    REQUIRE(
        running_read_providers[0U].service_state ==
        sovereign::ServiceState::Running);
    REQUIRE(
        running_read_providers[1U].service_state ==
        sovereign::ServiceState::Running);

    REQUIRE(host.StopAll());

    const auto stopped_read_providers =
        discovery.Providers("storage.read");

    REQUIRE(
        stopped_read_providers[0U].service_state ==
        sovereign::ServiceState::Stopped);
    REQUIRE(
        stopped_read_providers[1U].service_state ==
        sovereign::ServiceState::Stopped);

    auto removed =
        registry.Unregister("StoragePrimary");

    REQUIRE(removed != nullptr);

    REQUIRE(discovery.CapabilityCount() == 4U);

    const auto remaining_read_providers =
        discovery.Providers("storage.read");

    REQUIRE(
        remaining_read_providers.size() == 1U);
    REQUIRE(
        remaining_read_providers[0U]
            .service_name ==
        "StorageReplica");
    REQUIRE(
        remaining_read_providers[0U]
            .service_ordinal ==
        0U);

    REQUIRE(
        !discovery.HasCapability("storage.write"));

    return 0;
}
