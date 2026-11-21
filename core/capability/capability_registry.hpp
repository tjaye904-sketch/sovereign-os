#pragma once

#include "capability_types.hpp"

#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

namespace sovereign::capability
{

// Persistent allow/deny registry of system capabilities (L1.4).
class CapabilityRegistry
{
public:
    CapabilityError Open(const std::filesystem::path& data_root);
    void Close() noexcept;

    // Insert defaults if empty (idempotent).
    Result<std::size_t> SeedDefaults();

    Result<void> Register(const Capability& cap);
    Result<void> SetEnabled(const std::string& id, bool enabled);
    Result<Capability> Get(const std::string& id) const;
    Result<std::vector<Capability>> List() const;
    Result<bool> IsEnabled(const std::string& id) const;

    bool IsOpen() const noexcept { return open_; }

private:
    CapabilityError Load();
    CapabilityError Save() const;

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path path_;
    std::vector<Capability> items_;
};

} // namespace sovereign::capability
