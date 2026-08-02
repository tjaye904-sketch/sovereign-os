#pragma once

#include "runtime_state.hpp"

namespace sovereign
{

struct LifecycleTransition final
{
    RuntimeState previous{RuntimeState::Offline};

    RuntimeState requested{RuntimeState::Offline};

    RuntimeState current{RuntimeState::Offline};

    bool accepted{false};
};

} // namespace sovereign
