#pragma once

#include "config_value.hpp"

#include <cstddef>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace sovereign
{

class ConfigManager final
{
public:
    bool Set(
        std::string key,
        ConfigValue value);

    [[nodiscard]]
    bool Contains(
        std::string_view key) const;

    [[nodiscard]]
    std::optional<ConfigValue> Get(
        std::string_view key) const;

    bool Remove(
        std::string_view key);

    void Clear();

    [[nodiscard]]
    std::size_t Size() const;

private:
    mutable std::mutex mutex_;

    std::unordered_map<std::string, ConfigValue> values_;
};

} // namespace sovereign
