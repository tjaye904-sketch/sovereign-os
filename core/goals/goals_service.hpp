#pragma once

#include "goals_types.hpp"

#include <filesystem>
#include <mutex>
#include <vector>

namespace sovereign::goals
{

// Long-lived goals: the cognitive OS "what matters" register.
class GoalsService
{
public:
    GoalsError Open(const std::filesystem::path& directory);
    void Close() noexcept;

    Result<GoalId> Create(const Goal& goal);
    Result<Goal> Get(const GoalId& id) const;
    Result<std::vector<Goal>> ListActive() const;
    Result<std::vector<Goal>> ListAll() const;
    Result<void> SetStatus(const GoalId& id, GoalStatus status);
    Result<void> SetPriority(const GoalId& id, int priority);

private:
    GoalsError Load();
    GoalsError Save() const;
    static GoalId NewId();
    static std::uint64_t Now();

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path directory_;
    std::vector<Goal> goals_;
};

} // namespace sovereign::goals
