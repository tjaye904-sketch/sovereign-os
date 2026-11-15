#include "reasoning_engine.hpp"

#include <chrono>
#include <random>

namespace sovereign::cognitive
{

AnalysisId ReasoningEngine::NewId()
{
    AnalysisId id{};
    std::random_device rd;
    for (auto& byte : id.bytes)
    {
        byte = static_cast<std::uint8_t>(rd());
    }
    return id;
}

std::uint64_t ReasoningEngine::NowUnixSeconds()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

std::string ReasoningEngine::NormalizeQuestion(const std::string& value)
{
    std::string out = value;
    while (!out.empty() && (out.front() == ' ' || out.front() == '\n' || out.front() == '\r' || out.front() == '\t'))
        out.erase(out.begin());
    while (!out.empty() && (out.back() == ' ' || out.back() == '\n' || out.back() == '\r' || out.back() == '\t'))
        out.pop_back();
    return out;
}

ReasoningEngine::ReasoningEngine(sovereign::memory::MemoryEngine* memory)
    : memory_(memory)
{
}

Result<ReasoningResult> ReasoningEngine::Reason(
    const PrincipalId& requester,
    const std::string& question,
    const std::vector<std::string>& points_for,
    const std::vector<std::string>& points_against,
    const std::string& summary)
{
    Result<ReasoningResult> result;
    const std::string normalized = NormalizeQuestion(question);
    if (normalized.empty())
    {
        result.error = CognitiveError::InvalidArgument;
        return result;
    }

    ReasoningResult reason;
    reason.id = NewId();
    reason.requester = requester;
    reason.question = normalized;
    reason.points_for = points_for;
    reason.points_against = points_against;
    reason.summary = summary.empty() ? "Reasoning result captured; review evidence before acting." : summary;
    reason.created_unix_seconds = NowUnixSeconds();

    if (memory_ != nullptr)
    {
        sovereign::memory::MemoryRecord rec;
        rec.author = requester;
        rec.tags = {"cognitive", "reasoning"};
        rec.body = "question:" + reason.question + "|summary:" + reason.summary;
        rec.importance = 0.7;
        rec.created_unix_seconds = reason.created_unix_seconds;
        static_cast<void>(memory_->Append(rec));
    }

    result.value = reason;
    return result;
}

} // namespace sovereign::cognitive
