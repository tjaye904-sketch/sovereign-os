#include "runtime.hpp"

#include <cstdint>
#include <memory>
#include <string_view>
#include "test_require.hpp"

namespace
{

class RuntimeTestService final : public sovereign::Service
{
public:
    [[nodiscard]]
    std::string_view Name() const noexcept override
    {
        return "RuntimeTestService";
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
    sovereign::Runtime runtime;

    auto service =
        std::make_unique<RuntimeTestService>();

    RuntimeTestService* service_pointer =
        service.get();

    REQUIRE(runtime.RegisterService(
        std::move(service)));

    runtime.SetMaximumFrames(3U);

    REQUIRE(runtime.State() ==
           sovereign::RuntimeState::Offline);

    REQUIRE(runtime.Start());

    REQUIRE(runtime.State() ==
           sovereign::RuntimeState::Running);

    REQUIRE(service_pointer->start_count == 1U);

    REQUIRE(!runtime.RegisterService(
        std::make_unique<RuntimeTestService>()));

    REQUIRE(runtime.Run());

    REQUIRE(runtime.FrameCount() == 3U);
    REQUIRE(service_pointer->update_count == 3U);

    REQUIRE(runtime.Stop());

    REQUIRE(runtime.State() ==
           sovereign::RuntimeState::Stopped);

    REQUIRE(service_pointer->stop_count == 1U);

    return 0;
}
