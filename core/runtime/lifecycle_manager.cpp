#include "lifecycle_manager.hpp"

namespace sovereign
{

RuntimeState LifecycleManager::State() const noexcept
{
    std::scoped_lock lock(mutex_);

    return state_;
}

LifecycleTransition
LifecycleManager::LastTransition() const noexcept
{
    std::scoped_lock lock(mutex_);

    return last_transition_;
}

bool LifecycleManager::Transition(
    RuntimeState requested) noexcept
{
    std::scoped_lock lock(mutex_);

    last_transition_.previous = state_;
    last_transition_.requested = requested;
    last_transition_.current = state_;
    last_transition_.accepted = false;

    if (!Allowed(state_, requested))
    {
        return false;
    }

    state_ = requested;

    last_transition_.current = state_;
    last_transition_.accepted = true;

    return true;
}

bool LifecycleManager::Allowed(
    RuntimeState current,
    RuntimeState requested) noexcept
{
    switch (current)
    {
        case RuntimeState::Offline:
            return requested == RuntimeState::Initializing;

        case RuntimeState::Initializing:
            return requested == RuntimeState::Starting ||
                   requested == RuntimeState::Faulted;

        case RuntimeState::Starting:
            return requested == RuntimeState::Running ||
                   requested == RuntimeState::Faulted;

        case RuntimeState::Running:
            return requested == RuntimeState::Pausing ||
                   requested == RuntimeState::Stopping ||
                   requested == RuntimeState::Faulted;

        case RuntimeState::Pausing:
            return requested == RuntimeState::Paused ||
                   requested == RuntimeState::Faulted;

        case RuntimeState::Paused:
            return requested == RuntimeState::Resuming ||
                   requested == RuntimeState::Stopping ||
                   requested == RuntimeState::Faulted;

        case RuntimeState::Resuming:
            return requested == RuntimeState::Running ||
                   requested == RuntimeState::Faulted;

        case RuntimeState::Stopping:
            return requested == RuntimeState::Stopped ||
                   requested == RuntimeState::Faulted;

        case RuntimeState::Faulted:
            return requested == RuntimeState::Stopping;

        case RuntimeState::Stopped:
            return false;
    }

    return false;
}

} // namespace sovereign
