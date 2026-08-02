#pragma once

// Executes domain-allowed tools under host services.
// Tool calls never bypass Governance for consequential actions.

#include "domain_profile.hpp"
#include "memory_engine.hpp"
#include "semantic_store.hpp"

#include <string>

namespace sovereign::domain
{

struct ToolCallRequest
{
    std::string name;
    std::string arguments_json;
};

struct ToolCallResult
{
    bool allowed{false};
    bool ok{false};
    std::string detail;
};

class DomainToolRunner
{
public:
    DomainToolRunner(
        memory::MemoryEngine& memory,
        semantic::SemanticStore& semantic);

    ToolCallResult Run(
        const DomainProfile& profile,
        const ToolCallRequest& call);

private:
    memory::MemoryEngine& memory_;
    semantic::SemanticStore& semantic_;
};

} // namespace sovereign::domain
