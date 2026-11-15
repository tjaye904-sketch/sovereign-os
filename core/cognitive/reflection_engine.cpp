#include "reflection_engine.hpp"

#include <chrono>
#include <random>

namespace sovereign::cognitive
{

AnalysisId ReflectionEngine::NewId()
{
    AnalysisId id{};
    std::random_device rd;
    for (auto& byte : id.bytes)
    {
        byte = static_cast<std::uint8_t>(rd());
    }
    return id;
}

std::uint64_t ReflectionEngine::NowUnixSeconds()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

std::string ReflectionEngine::NormalizeText(const std::string& value)
{
    std::string out = value;
    while (!out.empty() && (out.front() == ' ' || out.front() == '\n' || out.front() == '\r' || out.front() == '\t'))
        out.erase(out.begin());
    while (!out.empty() && (out.back() == ' ' || out.back() == '\n' || out.back() == '\r' || out.back() == '\t'))
        out.pop_back();
    return out;
}

ReflectionEngine::ReflectionEngine(sovereign::memory::MemoryEngine* memory)
    : memory_(memory)
{
}

Result<ReflectionNote> ReflectionEngine::Reflect(
    const PrincipalId& author,
    const std::string& subject,
    const std::string& body)
{
    Result<ReflectionNote> result;
    const std::string clean_subject = NormalizeText(subject);
    const std::string clean_body = NormalizeText(body);
    if (clean_subject.empty() || clean_body.empty())
    {
        result.error = CognitiveError::InvalidArgument;
        return result;
    }

    ReflectionNote note;
    note.id = NewId();
    note.author = author;
    note.subject = clean_subject;
    note.body = clean_body;
    note.created_unix_seconds = NowUnixSeconds();

    if (memory_ != nullptr)
    {
        sovereign::memory::MemoryRecord rec;
        rec.author = author;
        rec.tags = {"cognitive", "reflection", clean_subject};
        rec.body = clean_body;
        rec.importance = 0.6;
        rec.created_unix_seconds = note.created_unix_seconds;
        auto app = memory_->Append(rec);
        if (!app.Ok())
        {
            result.error = CognitiveError::StorageFailure;
            return result;
        }
    }

    result.value = note;
    return result;
}

} // namespace sovereign::cognitive
