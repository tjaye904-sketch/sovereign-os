#!/usr/bin/env python3
"""L1.3: Suspended status + Activate/Suspend/FindByName on WorkspaceService."""
from pathlib import Path
import sys

root = Path(sys.argv[1])
types = root / "core/workspace/workspace_types.hpp"
svc_h = root / "core/workspace/workspace_service.hpp"
svc_c = root / "core/workspace/workspace_service.cpp"

t = types.read_text()
if "Suspended" not in t:
    t = t.replace(
        "enum class WorkspaceStatus : std::uint8_t\n{\n    Active = 1,\n    Archived = 2\n};",
        "enum class WorkspaceStatus : std::uint8_t\n{\n    Active = 1,\n    Archived = 2,\n    Suspended = 3  // L1.3 controller — logical pause (no VM yet)\n};",
    )
    types.write_text(t)
    print("types: Suspended")
else:
    print("types: Suspended already")

h = svc_h.read_text()
if "Suspend" not in h:
    h = h.replace(
        "Result<void> Archive(const WorkspaceId& id);",
        """Result<void> Archive(const WorkspaceId& id);
    // L1.3 Workspace Controller MVP (logical lifecycle — no node/VM start)
    Result<void> Activate(const WorkspaceId& id);
    Result<void> Suspend(const WorkspaceId& id);
    Result<Workspace> FindByName(const std::string& name) const;
    Result<std::vector<Workspace>> ListByStatus(WorkspaceStatus status) const;
    static const char* StatusName(WorkspaceStatus s);""",
    )
    svc_h.write_text(h)
    print("hpp: Activate/Suspend")
else:
    print("hpp: already")

c = svc_c.read_text()
if "WorkspaceService::Suspend" in c:
    print("cpp: already")
else:
    methods = r'''
const char* WorkspaceService::StatusName(WorkspaceStatus s)
{
    switch (s)
    {
    case WorkspaceStatus::Active: return "active";
    case WorkspaceStatus::Archived: return "archived";
    case WorkspaceStatus::Suspended: return "suspended";
    }
    return "unknown";
}

Result<void> WorkspaceService::Activate(const WorkspaceId& id)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& w : workspaces_)
    {
        if (w.id == id)
        {
            if (w.status == WorkspaceStatus::Archived)
            {
                result.error = WorkspaceError::InvalidArgument;
                return result;
            }
            w.status = WorkspaceStatus::Active;
            if (Save() != WorkspaceError::Ok)
            {
                result.error = WorkspaceError::StorageFailure;
                return result;
            }
            return result;
        }
    }
    result.error = WorkspaceError::NotFound;
    return result;
}

Result<void> WorkspaceService::Suspend(const WorkspaceId& id)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& w : workspaces_)
    {
        if (w.id == id)
        {
            if (w.status == WorkspaceStatus::Archived)
            {
                result.error = WorkspaceError::InvalidArgument;
                return result;
            }
            w.status = WorkspaceStatus::Suspended;
            if (Save() != WorkspaceError::Ok)
            {
                result.error = WorkspaceError::StorageFailure;
                return result;
            }
            return result;
        }
    }
    result.error = WorkspaceError::NotFound;
    return result;
}

Result<Workspace> WorkspaceService::FindByName(const std::string& name) const
{
    std::lock_guard lock(mutex_);
    Result<Workspace> result;
    for (const auto& w : workspaces_)
    {
        if (w.name == name)
        {
            result.value = w;
            return result;
        }
    }
    result.error = WorkspaceError::NotFound;
    return result;
}

Result<std::vector<Workspace>> WorkspaceService::ListByStatus(WorkspaceStatus status) const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<Workspace>> result;
    for (const auto& w : workspaces_)
    {
        if (w.status == status)
            result.value.push_back(w);
    }
    return result;
}

'''
    c = c.replace("} // namespace sovereign::workspace", methods + "} // namespace sovereign::workspace")
    svc_c.write_text(c)
    print("cpp: methods added")
