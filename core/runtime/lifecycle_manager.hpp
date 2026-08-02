#pragma once

#include "lifecycle_transition.hpp"
#include "runtime_state.hpp"

#include <mutex>

namespace sovereign
{

class LifecycleManager final
{
public:
    [[nodiscard]]
    RuntimeState State() const noexcept;

    [[nodiscard]]
    LifecycleTransition LastTransition() const noexcept;

    bool Transition(RuntimeState requested) noexcept;

private:
    [[nodiscard]]
    static bool Allowed(
        RuntimeState current,
        RuntimeState requested) noexcept;

    mutable std::mutex mutex_;

    RuntimeState state_{RuntimeState::Offline};

    LifecycleTransition last_transition_;
};

} // namespace sovereign
