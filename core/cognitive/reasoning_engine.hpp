#pragma once

#include "cognitive_types.hpp"
#include "memory_engine.hpp"

#include <string>
#include <vector>

namespace sovereign::cognitive
{

class ReasoningEngine
{
public:
    explicit ReasoningEngine(sovereign::memory::MemoryEngine* memory = nullptr);

    Result<ReasoningResult> Reason(
        const PrincipalId& requester,
        const std::string& question,
        const std::vector<std::string>& points_for,
        const std::vector<std::string>& points_against,
        const std::string& summary);

private:
    static std::string NormalizeQuestion(const std::string& value);
    static std::uint64_t NowUnixSeconds();
    static AnalysisId NewId();

    sovereign::memory::MemoryEngine* memory_;
};

} // namespace sovereign::cognitive
