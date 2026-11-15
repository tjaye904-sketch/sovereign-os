#pragma once

#include "cognitive_types.hpp"
#include "memory_engine.hpp"

#include <string>

namespace sovereign::cognitive
{

class ReflectionEngine
{
public:
    explicit ReflectionEngine(sovereign::memory::MemoryEngine* memory = nullptr);

    Result<ReflectionNote> Reflect(
        const PrincipalId& author,
        const std::string& subject,
        const std::string& body);

private:
    static std::string NormalizeText(const std::string& value);
    static AnalysisId NewId();
    static std::uint64_t NowUnixSeconds();

    sovereign::memory::MemoryEngine* memory_;
};

} // namespace sovereign::cognitive
