#include "config_manager.hpp"

#include <utility>

namespace sovereign
{

bool ConfigManager::Set(
    std::string key,
    ConfigValue value)
{
    if (key.empty())
    {
        return false;
    }

    std::scoped_lock lock(mutex_);

    values_.insert_or_assign(
        std::move(key),
        std::move(value));

    return true;
}

bool ConfigManager::Contains(
    std::string_view key) const
{
    std::scoped_lock lock(mutex_);

    return values_.contains(
        std::string(key));
}

std::optional<ConfigValue>
ConfigManager::Get(
    std::string_view key) const
{
    std::scoped_lock lock(mutex_);

    const auto iterator =
        values_.find(std::string(key));

    if (iterator == values_.end())
    {
        return std::nullopt;
    }

    return iterator->second;
}

bool ConfigManager::Remove(
    std::string_view key)
{
    std::scoped_lock lock(mutex_);

    return values_.erase(
        std::string(key)) > 0U;
}

void ConfigManager::Clear()
{
    std::scoped_lock lock(mutex_);

    values_.clear();
}

std::size_t ConfigManager::Size() const
{
    std::scoped_lock lock(mutex_);

    return values_.size();
}

} // namespace sovereign
