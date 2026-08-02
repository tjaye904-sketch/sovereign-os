#pragma once

#include "frame_context.hpp"
#include "service_state.hpp"

#include <string_view>
#include <vector>

namespace sovereign
{

class EventBus;
class ServiceHost;

class Service
{
public:
    virtual ~Service() = default;

    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;

    Service(Service&&) = delete;
    Service& operator=(Service&&) = delete;

    [[nodiscard]]
    virtual std::string_view Name() const noexcept = 0;

    [[nodiscard]]
    virtual std::vector<std::string_view>
    Dependencies() const
    {
        return {};
    }

    [[nodiscard]]
    virtual std::vector<std::string_view>
    Capabilities() const
    {
        return {};
    }

    virtual bool Start(EventBus& event_bus) = 0;

    virtual bool Update(const FrameContext& frame) = 0;

    virtual bool Stop() = 0;

    [[nodiscard]]
    ServiceState State() const noexcept
    {
        return state_;
    }

protected:
    Service() = default;

private:
    friend class ServiceHost;

    void SetState(ServiceState state) noexcept
    {
        state_ = state;
    }

    ServiceState state_{ServiceState::Created};
};

} // namespace sovereign
