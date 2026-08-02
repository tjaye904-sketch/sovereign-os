#pragma once

// Local-first sync journal: records export/import intents for later peers.
// v1 does not network; it appends journal entries for offline reconciliation.

#include "principal_id.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

namespace sovereign::sync
{

using ::sovereign::PrincipalId;

struct SyncEntryId
{
    std::array<std::uint8_t, 16> bytes{};
};

enum class SyncDirection : std::uint8_t
{
    Export = 1,
    Import = 2
};

struct SyncEntry
{
    SyncEntryId id{};
    PrincipalId actor{};
    SyncDirection direction{SyncDirection::Export};
    std::string channel; // e.g. "usb", "folder", "future-peer"
    std::string payload_ref;
    std::uint64_t created_unix_seconds{0};
};

enum class SyncError : std::uint8_t
{
    Ok = 0,
    InvalidArgument,
    StorageFailure,
    NotFound
};

template <typename T>
struct Result
{
    SyncError error{SyncError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == SyncError::Ok;
    }
};

class SyncService
{
public:
    SyncError Open(const std::filesystem::path& directory);
    void Close() noexcept;

    Result<SyncEntryId> RecordExport(
        const PrincipalId& actor,
        const std::string& channel,
        const std::string& payload_ref);

    Result<SyncEntryId> RecordImport(
        const PrincipalId& actor,
        const std::string& channel,
        const std::string& payload_ref);

    Result<std::vector<SyncEntry>> List() const;

private:
    SyncError Load();
    SyncError Save() const;
    static SyncEntryId NewId();
    static std::uint64_t Now();
    Result<SyncEntryId> Append(const SyncEntry& entry);

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path directory_;
    std::vector<SyncEntry> entries_;
};

} // namespace sovereign::sync
