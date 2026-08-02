#pragma once

// Sovereign completion rule: full operational transparency.
// A single report answers identity, memory, model, policy, plans,
// approvals, executions, state, timing, errors, audit, health, coverage.

#include "sovereign_host.hpp"

#include <chrono>
#include <string>

namespace sovereign::diagnostics
{

struct TransparencyReport
{
    std::string text;
};

class TransparencyService
{
public:
    explicit TransparencyService(product::SovereignHost& host);

    // Full report covering all completion-rule sections.
    TransparencyReport BuildFull() const;

    // Section filter: identity|memory|model|policy|plans|approvals|
    // executions|state|timing|errors|audit|health|coverage|all
    TransparencyReport BuildSection(const std::string& section) const;

private:
    product::SovereignHost& host_;
    std::chrono::steady_clock::time_point started_;
};

} // namespace sovereign::diagnostics
