#include "event_bus.hpp"
#include "service_dependency.hpp"
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

class DependencyTestService final
    : public sovereign::Service
{
public:
    DependencyTestService(
        std::string name,
        std::vector<std::string_view> dependencies,
        std::vector<std::string>& lifecycle_log)
        :
        name_(std::move(name)),
        dependencies_(std::move(dependencies)),
        lifecycle_log_(lifecycle_log)
    {
    }

    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return name_;
    }

    [[nodiscard]]
    std::vector<std::string_view>
    Dependencies() const override
    {
        return dependencies_;
    }

    bool Start(sovereign::EventBus&) override
    {
        lifecycle_log_.push_back(
            "start:" + name_);

        return true;
    }

    bool Update(
        const sovereign::FrameContext&) override
    {
        lifecycle_log_.push_back(
            "update:" + name_);

        return true;
    }

    bool Stop() override
    {
        lifecycle_log_.push_back(
            "stop:" + name_);

        return true;
    }

private:
    std::string name_;
    std::vector<std::string_view> dependencies_;
    std::vector<std::string>& lifecycle_log_;
};

} // namespace

int main()
{
    {
        sovereign::EventBus event_bus;
        sovereign::ServiceRegistry registry;
        sovereign::ServiceHost host(
            event_bus,
            registry);

        std::vector<std::string> lifecycle_log{};

        REQUIRE(registry.Register(
            std::make_unique<
                DependencyTestService>(
                    "Application",
                    std::vector<std::string_view>{
                        "Storage",
                        "Messaging"},
                    lifecycle_log)));

        REQUIRE(registry.Register(
            std::make_unique<
                DependencyTestService>(
                    "Messaging",
                    std::vector<std::string_view>{
                        "Storage"},
                    lifecycle_log)));

        REQUIRE(registry.Register(
            std::make_unique<
                DependencyTestService>(
                    "Storage",
                    std::vector<std::string_view>{},
                    lifecycle_log)));

        REQUIRE(host.StartAll());

        const auto& resolution =
            host.LastDependencyResolution();

        REQUIRE(resolution.Ready());
        REQUIRE(
            resolution.status ==
            sovereign::ServiceDependencyStatus::
                Ready);
        REQUIRE(resolution.startup_order.size() == 3U);

        REQUIRE(
            resolution.startup_order[0U]->Name() ==
            "Storage");
        REQUIRE(
            resolution.startup_order[1U]->Name() ==
            "Messaging");
        REQUIRE(
            resolution.startup_order[2U]->Name() ==
            "Application");

        REQUIRE(lifecycle_log.size() == 3U);
        REQUIRE(lifecycle_log[0U] == "start:Storage");
        REQUIRE(lifecycle_log[1U] == "start:Messaging");
        REQUIRE(
            lifecycle_log[2U] ==
            "start:Application");

        sovereign::FrameContext frame{};

        REQUIRE(host.UpdateAll(frame));

        REQUIRE(lifecycle_log[3U] == "update:Storage");
        REQUIRE(
            lifecycle_log[4U] ==
            "update:Messaging");
        REQUIRE(
            lifecycle_log[5U] ==
            "update:Application");

        REQUIRE(host.StopAll());

        REQUIRE(
            lifecycle_log[6U] ==
            "stop:Application");
        REQUIRE(
            lifecycle_log[7U] ==
            "stop:Messaging");
        REQUIRE(lifecycle_log[8U] == "stop:Storage");
    }

    {
        sovereign::EventBus event_bus;
        sovereign::ServiceRegistry registry;
        sovereign::ServiceHost host(
            event_bus,
            registry);

        std::vector<std::string> lifecycle_log{};

        REQUIRE(registry.Register(
            std::make_unique<
                DependencyTestService>(
                    "Application",
                    std::vector<std::string_view>{
                        "MissingStorage"},
                    lifecycle_log)));

        REQUIRE(!host.StartAll());

        const auto& resolution =
            host.LastDependencyResolution();

        REQUIRE(
            resolution.status ==
            sovereign::ServiceDependencyStatus::
                MissingDependency);

        REQUIRE(
            resolution.missing_dependencies.size() ==
            1U);

        REQUIRE(
            resolution
                .missing_dependencies[0U]
                .service_name ==
            "Application");

        REQUIRE(
            resolution
                .missing_dependencies[0U]
                .dependency_name ==
            "MissingStorage");

        REQUIRE(lifecycle_log.empty());
    }

    {
        sovereign::EventBus event_bus;
        sovereign::ServiceRegistry registry;
        sovereign::ServiceHost host(
            event_bus,
            registry);

        std::vector<std::string> lifecycle_log{};

        REQUIRE(registry.Register(
            std::make_unique<
                DependencyTestService>(
                    "Alpha",
                    std::vector<std::string_view>{
                        "Beta"},
                    lifecycle_log)));

        REQUIRE(registry.Register(
            std::make_unique<
                DependencyTestService>(
                    "Beta",
                    std::vector<std::string_view>{
                        "Alpha"},
                    lifecycle_log)));

        REQUIRE(!host.StartAll());

        const auto& resolution =
            host.LastDependencyResolution();

        REQUIRE(
            resolution.status ==
            sovereign::ServiceDependencyStatus::
                DependencyCycle);

        REQUIRE(resolution.cycle_services.size() == 2U);
        REQUIRE(resolution.cycle_services[0U] == "Alpha");
        REQUIRE(resolution.cycle_services[1U] == "Beta");
        REQUIRE(lifecycle_log.empty());
    }

    return 0;
}
