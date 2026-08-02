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
