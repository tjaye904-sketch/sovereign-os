#pragma once

// Sovereign completion rule: full operational transparency.
// A single report answers identity, memory, model, policy, plans,
// approvals, executions, state, timing, errors, audit, health, coverage.

#include "sovereign_host.hpp"

#include <chrono>
#include <string>
#include <vector>

namespace sovereign::diagnostics
{

struct TransparencyReport
{
    std::string text;
};

struct DoctorCheck
{
    std::string name;
    bool ok{false};
    std::string detail;
};

struct DoctorReport
{
    bool all_ok{false};
    std::size_t passed{0};
    std::size_t total{0};
    std::vector<DoctorCheck> checks;
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
    DoctorReport RunDoctor(bool auto_fix = false);

private:
    product::SovereignHost& host_;
    std::chrono::steady_clock::time_point started_;
};

} // namespace sovereign::diagnostics
