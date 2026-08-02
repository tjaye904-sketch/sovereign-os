#pragma once

// Aggregates durable system state into a single coherent brief —
// the "how are things today?" surface of the cognitive OS.

#include "audit_log.hpp"
#include "goals_service.hpp"
#include "governance_service.hpp"
#include "memory_engine.hpp"
#include "workspace_service.hpp"

#include <string>
#include <vector>

namespace sovereign::briefing
{

struct BriefSection
{
    std::string title;
    std::vector<std::string> lines;
};

struct SystemBrief
{
    std::vector<BriefSection> sections;
    std::string rendered_text;
};

class BriefingService
{
public:
    BriefingService(
        goals::GoalsService& goals,
        governance::GovernanceService& governance,
        memory::MemoryEngine& memory,
        workspace::WorkspaceService& workspace,
        audit::AuditLog& audit);

    SystemBrief Build() const;

private:
    goals::GoalsService& goals_;
    governance::GovernanceService& governance_;
    memory::MemoryEngine& memory_;
    workspace::WorkspaceService& workspace_;
    audit::AuditLog& audit_;
};

} // namespace sovereign::briefing
