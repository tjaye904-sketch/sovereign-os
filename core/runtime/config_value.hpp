#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace sovereign
{

using ConfigValue =
    std::variant<
        bool,
        std::int64_t,
        double,
        std::string>;

} // namespace sovereign
