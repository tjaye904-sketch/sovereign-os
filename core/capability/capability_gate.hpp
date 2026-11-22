#pragma once

#include "capability_registry.hpp"

#include <string>
#include <utility>

namespace sovereign::capability
{

// L3.1 — gate an action behind an enabled capability id.
struct GateDecision
{
    bool allowed{false};
    std::string capability_id;
    std::string reason;
};

class CapabilityGate
{
public:
    explicit CapabilityGate(CapabilityRegistry& registry) : reg_(registry) {}

    // Map high-level action names to capability ids (L3.4).
    static std::string CapabilityForAction(const std::string& action);

    GateDecision Require(const std::string& capability_id);
    GateDecision RequireForAction(const std::string& action);

private:
    CapabilityRegistry& reg_;
};

} // namespace sovereign::capability
