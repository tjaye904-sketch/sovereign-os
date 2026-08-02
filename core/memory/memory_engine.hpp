#pragma once

#include "memory_types.hpp"

#include <filesystem>
#include <mutex>
#include <vector>

namespace sovereign::memory
{

class MemoryEngine
{
public:
    MemoryError Open(const std::filesystem::path& directory);
    void Close() noexcept;

    Result<RecordId> Append(const MemoryRecord& record);
    Result<MemoryRecord> Get(const RecordId& id) const;
    Result<std::vector<MemoryRecord>> ListByTag(const std::string& tag) const;
    Result<std::vector<MemoryRecord>> ListAll() const;
    Result<std::vector<MemoryRecord>> Search(const std::string& query) const;

    // Phase V — importance & aging
    Result<void> Touch(const RecordId& id); // bump access
    Result<void> SetImportance(const RecordId& id, double importance);
    Result<std::vector<ScoredRecord>> Ranked(std::size_t limit = 50) const;
    // Remove records with aged score below threshold (does not delete if score ok)
    Result<std::uint32_t> PruneBelow(double min_score);

    static double AgeScore(const MemoryRecord& r, std::uint64_t now_unix);

private:
    MemoryError Load();
    MemoryError Save() const;
    static RecordId NewId();
    static std::uint64_t Now();

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path directory_;
    std::vector<MemoryRecord> records_;
};

} // namespace sovereign::memory
