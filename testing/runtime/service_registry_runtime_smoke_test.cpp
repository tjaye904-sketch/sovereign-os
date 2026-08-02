#include "event_bus.hpp"
#include "service_host.hpp"
#include "service_registry.hpp"

#include <cstdint>
#include <memory>
#include <string_view>
#include "test_require.hpp"

namespace
{

class RegistryTestService final
    : public sovereign::Service
{
public:
    explicit RegistryTestService(
        std::string_view name)
        :
        name_(name)
    {
    }

    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return name_;
    }

    bool Start(sovereign::EventBus&) override
    {
        ++start_count;
        return true;
    }

    bool Update(
        const sovereign::FrameContext&) override
    {
        ++update_count;
        return true;
    }

    bool Stop() override
    {
        ++stop_count;
        return true;
    }

    std::uint64_t start_count{0U};
    std::uint64_t update_count{0U};
    std::uint64_t stop_count{0U};

private:
    std::string_view name_;
};

} // namespace

int main()
{
    sovereign::ServiceRegistry registry;

    REQUIRE(registry.Empty());
    REQUIRE(registry.Size() == 0U);
    REQUIRE(registry.Generation() == 0U);

    const auto null_result =
        registry.RegisterDetailed(nullptr);

    REQUIRE(!null_result.Accepted());
    REQUIRE(
        null_result.status ==
        sovereign::ServiceRegistrationStatus::
            NullService);
    REQUIRE(null_result.size_before == 0U);
    REQUIRE(null_result.size_after == 0U);
    REQUIRE(registry.Generation() == 0U);

    const auto empty_result =
        registry.RegisterDetailed(
            std::make_unique<
                RegistryTestService>(""));

    REQUIRE(!empty_result.Accepted());
    REQUIRE(
        empty_result.status ==
        sovereign::ServiceRegistrationStatus::
            EmptyName);
    REQUIRE(registry.Empty());
    REQUIRE(registry.Generation() == 0U);

    auto alpha =
        std::make_unique<
            RegistryTestService>("Alpha");

    RegistryTestService* alpha_pointer =
        alpha.get();

    const auto alpha_result =
        registry.RegisterDetailed(
            std::move(alpha));

    REQUIRE(alpha_result.Accepted());
    REQUIRE(alpha_result.service_name == "Alpha");
    REQUIRE(alpha_result.ordinal == 0U);
    REQUIRE(alpha_result.size_before == 0U);
    REQUIRE(alpha_result.size_after == 1U);
    REQUIRE(registry.Generation() == 1U);

    REQUIRE(
        registry.Register(
            std::make_unique<
                RegistryTestService>("Beta")));

    REQUIRE(registry.Size() == 2U);
    REQUIRE(registry.Generation() == 2U);
    REQUIRE(registry.Contains("Alpha"));
    REQUIRE(registry.Contains("Beta"));
    REQUIRE(!registry.Contains("Missing"));
    REQUIRE(registry.Find("Alpha") == alpha_pointer);

    const auto duplicate_result =
        registry.RegisterDetailed(
            std::make_unique<
                RegistryTestService>("Alpha"));

    REQUIRE(!duplicate_result.Accepted());
    REQUIRE(
        duplicate_result.status ==
        sovereign::ServiceRegistrationStatus::
            DuplicateName);
    REQUIRE(duplicate_result.service_name == "Alpha");
    REQUIRE(duplicate_result.size_before == 2U);
    REQUIRE(duplicate_result.size_after == 2U);
    REQUIRE(registry.Generation() == 2U);

    const auto& last_registration =
        registry.LastRegistration();

    REQUIRE(
        last_registration.status ==
        sovereign::ServiceRegistrationStatus::
            DuplicateName);

    const auto initial_inventory =
        registry.Inventory();

    REQUIRE(initial_inventory.size() == 2U);

    REQUIRE(initial_inventory[0U].name == "Alpha");
    REQUIRE(initial_inventory[0U].ordinal == 0U);
    REQUIRE(
        initial_inventory[0U].state ==
        sovereign::ServiceState::Created);

    REQUIRE(initial_inventory[1U].name == "Beta");
    REQUIRE(initial_inventory[1U].ordinal == 1U);
    REQUIRE(
        initial_inventory[1U].state ==
        sovereign::ServiceState::Created);

    sovereign::EventBus event_bus;
    sovereign::ServiceHost host(event_bus, registry);

    REQUIRE(host.StartAll());

    REQUIRE(
        alpha_pointer->State() ==
        sovereign::ServiceState::Running);

    REQUIRE(registry.Unregister("Alpha") == nullptr);
    REQUIRE(registry.Contains("Alpha"));
    REQUIRE(registry.Generation() == 2U);

    const auto running_inventory =
        registry.Inventory();

    REQUIRE(
        running_inventory[0U].state ==
        sovereign::ServiceState::Running);
    REQUIRE(
        running_inventory[1U].state ==
        sovereign::ServiceState::Running);

    REQUIRE(host.StopAll());

    auto removed_alpha =
        registry.Unregister("Alpha");

    REQUIRE(removed_alpha != nullptr);
    REQUIRE(removed_alpha.get() == alpha_pointer);
    REQUIRE(
        removed_alpha->State() ==
        sovereign::ServiceState::Stopped);

    REQUIRE(!registry.Contains("Alpha"));
    REQUIRE(registry.Size() == 1U);
    REQUIRE(registry.Generation() == 3U);

    REQUIRE(registry.Unregister("Missing") == nullptr);
    REQUIRE(registry.Generation() == 3U);

    const auto final_inventory =
        registry.Inventory();

    REQUIRE(final_inventory.size() == 1U);
    REQUIRE(final_inventory[0U].name == "Beta");
    REQUIRE(final_inventory[0U].ordinal == 0U);
    REQUIRE(
        final_inventory[0U].state ==
        sovereign::ServiceState::Stopped);

    auto removed_beta =
        registry.Unregister("Beta");

    REQUIRE(removed_beta != nullptr);
    REQUIRE(registry.Empty());
    REQUIRE(registry.Generation() == 4U);

    return 0;
}
