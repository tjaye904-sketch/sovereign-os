#pragma once

#include "workspace_types.hpp"

#include <filesystem>
#include <mutex>
#include <vector>

namespace sovereign::workspace
{

class WorkspaceService
{
public:
    WorkspaceError Open(const std::filesystem::path& directory);
    void Close() noexcept;

    Result<WorkspaceId> Create(const std::string& name, const std::string& kind);
    Result<Workspace> Get(const WorkspaceId& id) const;
    Result<std::vector<Workspace>> List() const;
    Result<void> Archive(const WorkspaceId& id);
    // L1.3 Workspace Controller MVP (logical lifecycle — no node/VM start)
    Result<void> Activate(const WorkspaceId& id);
    Result<void> Suspend(const WorkspaceId& id);
    Result<Workspace> FindByName(const std::string& name) const;
    Result<std::vector<Workspace>> ListByStatus(WorkspaceStatus status) const;
    static const char* StatusName(WorkspaceStatus s);

private:
    WorkspaceError Load();
    WorkspaceError Save() const;
    static WorkspaceId NewId();
    static std::uint64_t Now();

    mutable std::mutex mutex_;
    bool open_{false};
    std::filesystem::path directory_;
    std::vector<Workspace> workspaces_;
};

} // namespace sovereign::workspace
