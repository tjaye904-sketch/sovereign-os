#include "event_bus.hpp"
#include "service_host.hpp"
#include "service_registry.hpp"

#include <cstdint>
#include <memory>
#include <string_view>
#include "test_require.hpp"

namespace
{

class TestService final : public sovereign::Service
{
public:
    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return "TestService";
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

    std::uint64_t start_count{0};
    std::uint64_t update_count{0};
    std::uint64_t stop_count{0};
};

} // namespace

int main()
{
    sovereign::EventBus event_bus;
    sovereign::ServiceRegistry registry;
    sovereign::ServiceHost host(event_bus, registry);

    auto service =
        std::make_unique<TestService>();

    TestService* service_pointer =
        service.get();

    REQUIRE(registry.Register(std::move(service)));
    REQUIRE(!registry.Register(nullptr));
    REQUIRE(registry.Size() == 1U);
    REQUIRE(!registry.Empty());
    REQUIRE(registry.Find("TestService") == service_pointer);
    REQUIRE(registry.Find("MissingService") == nullptr);

    REQUIRE(service_pointer->State() ==
           sovereign::ServiceState::Created);

    REQUIRE(host.StartAll());
    REQUIRE(service_pointer->State() ==
           sovereign::ServiceState::Running);
    REQUIRE(service_pointer->start_count == 1U);

    sovereign::FrameContext frame;
    frame.frame_number = 0U;

    REQUIRE(host.UpdateAll(frame));
    REQUIRE(service_pointer->update_count == 1U);

    REQUIRE(host.StopAll());
    REQUIRE(service_pointer->State() ==
           sovereign::ServiceState::Stopped);
    REQUIRE(service_pointer->stop_count == 1U);

    return 0;
}
