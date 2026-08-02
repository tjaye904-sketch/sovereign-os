#pragma once

#include "config_value.hpp"

#include <string>

namespace sovereign
{

struct ConfigEntry final
{
    std::string key;

    ConfigValue value;
};

} // namespace sovereign
