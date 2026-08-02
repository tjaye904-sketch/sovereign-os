#pragma once

#include "audit_types.hpp"

#include <filesystem>
#include <mutex>
#include <vector>

namespace sovereign::audit
{

// Append-only local audit log (product DoD for identity/governance events).
// Does not carry secrets. File-backed under data_root/audit.
class AuditLog
{
public:
    AuditError Open(const std::filesystem::path& directory);
    void Close() noexcept;

    Result<AuditEntryId> Append(const AuditEntry& entry);
    Result<std::vector<AuditEntry>> List(std::size_t limit = 100) const;
    Result<std::vector<AuditEntry>> ListByCategory(
        const std::string& category,
        std::size_t limit = 100) const;

private:
    AuditError Load();
    AuditError AppendLine(const AuditEntry& entry) const;
    static AuditEntryId NewId();
    static std::uint64_t Now();

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path directory_;
    std::vector<AuditEntry> entries_;
};

} // namespace sovereign::audit
