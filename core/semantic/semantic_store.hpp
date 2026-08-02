#pragma once

#include "semantic_types.hpp"

#include <filesystem>
#include <mutex>
#include <vector>

namespace sovereign::semantic
{

class SemanticStore
{
public:
    SemanticError Open(const std::filesystem::path& directory);
    void Close() noexcept;

    Result<SemanticRecordId> Upsert(const std::string& text,
                                    const std::vector<std::string>& tags,
                                    const PrincipalId& author = {});
    Result<std::vector<ScoredRecord>> Query(const std::string& query,
                                            std::size_t top_k = 5) const;
    Result<std::size_t> Size() const;

private:
    SemanticError Load();
    SemanticError Save() const;
    static SemanticRecordId NewId();
    static std::uint64_t Now();

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path directory_;
    std::vector<SemanticRecord> records_;
};

} // namespace sovereign::semantic
