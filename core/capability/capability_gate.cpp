#include "capability_gate.hpp"

namespace sovereign::capability
{

std::string CapabilityGate::CapabilityForAction(const std::string& action)
{
    // L3.2–L3.5 action → capability map
    if (action == "execute" || action == "executor" || action == "governed_execute")
        return "host.execute";
    if (action == "purpose" || action == "purpose-run" || action == "purpose_run")
        return "host.purpose";
    if (action == "computer-use" || action == "browser" || action == "browser_open"
        || action == "cu")
        return "host.computer_use";
    if (action == "sys" || action == "app_open" || action == "desktop")
        return "host.sys";
    if (action == "voice" || action == "speak" || action == "listen")
        return "host.voice";
    if (action == "chat" || action == "model" || action == "plan")
        return "model.chat";
    if (action == "memory" || action == "memory-add")
        return "memory.write";
    // Unknown action: deny by requiring a non-seeded id (fail closed on check).
    return "action." + action;
}

GateDecision CapabilityGate::Require(const std::string& capability_id)
{
    GateDecision d;
    d.capability_id = capability_id;
    if (capability_id.empty())
    {
        d.allowed = false;
        d.reason = "empty capability id";
        return d;
    }
    auto r = reg_.IsEnabled(capability_id);
    if (!r.Ok())
    {
        d.allowed = false;
        d.reason = "capability not found or registry error: " + capability_id;
        return d;
    }
    if (!r.value)
    {
        d.allowed = false;
        d.reason = "capability disabled: " + capability_id;
        return d;
    }
    d.allowed = true;
    d.reason = "allowed";
    return d;
}

GateDecision CapabilityGate::RequireForAction(const std::string& action)
{
    return Require(CapabilityForAction(action));
}

} // namespace sovereign::capability
