#include "curiosity_engine.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <random>

namespace sovereign::cognitive
{
namespace
{
std::string Trim(const std::string& value)
{
    std::string out = value;
    while (!out.empty() && (out.front() == ' ' || out.front() == '\t' ||
                           out.front() == '\n' || out.front() == '\r'))
    {
        out.erase(out.begin());
    }
    while (!out.empty() && (out.back() == ' ' || out.back() == '\t' ||
                           out.back() == '\n' || out.back() == '\r'))
    {
        out.pop_back();
    }
    return out;
}

std::string ReadValueAfterPrefix(const std::string& body, const std::string& prefix)
{
    if (body.rfind(prefix, 0) != 0)
        return {};
    return body.substr(prefix.size());
}

std::uint64_t NowUnixSeconds()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

} // namespace

AnalysisId CuriosityEngine::NewId()
{
    AnalysisId id{};
    std::random_device rd;
    for (auto& b : id.bytes)
    {
        b = static_cast<std::uint8_t>(rd());
    }
    return id;
}

std::uint64_t CuriosityEngine::Now()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

std::string CuriosityEngine::NormalizeTopic(const std::string& topic)
{
    return Trim(topic);
}

std::string BackgroundDiscoveryQueue::NormalizeTopic(const std::string& topic)
{
    return Trim(topic);
}

void BackgroundDiscoveryQueue::Enqueue(
    const std::string& topic,
    double priority,
    std::uint64_t now)
{
    const std::string normalized = NormalizeTopic(topic);
    if (normalized.empty())
    {
        return;
    }

    const std::uint64_t current_time = now == 0 ? NowUnixSeconds() : now;
    for (auto& entry : entries_)
    {
        if (entry.topic == normalized)
        {
            entry.priority = std::max(entry.priority, priority);
            entry.attempts += 1;
            entry.next_run_unix_seconds = current_time + 60;
            return;
        }
    }

    DiscoveryQueueEntry entry;
    entry.topic = normalized;
    entry.priority = priority;
    entry.created_unix_seconds = current_time;
    entry.next_run_unix_seconds = current_time + 60;
    entry.attempts = 1;
    entries_.push_back(entry);
}

std::vector<DiscoveryQueueEntry> BackgroundDiscoveryQueue::Snapshot() const
{
    std::vector<DiscoveryQueueEntry> copy = entries_;
    std::sort(copy.begin(), copy.end(), [](const DiscoveryQueueEntry& lhs, const DiscoveryQueueEntry& rhs) {
        if (lhs.priority != rhs.priority)
            return lhs.priority > rhs.priority;
        return lhs.created_unix_seconds < rhs.created_unix_seconds;
    });
    return copy;
}

std::vector<DiscoveryQueueEntry> BackgroundDiscoveryQueue::PopReady(
    std::uint64_t now,
    std::size_t max)
{
    std::vector<DiscoveryQueueEntry> ready;
    auto it = entries_.begin();
    while (it != entries_.end() && ready.size() < max)
    {
        if (it->next_run_unix_seconds <= now)
        {
            ready.push_back(*it);
            it = entries_.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return ready;
}

bool BackgroundDiscoveryQueue::Empty() const noexcept
{
    return entries_.empty();
}

MemoryBackedInterestGraph::MemoryBackedInterestGraph(
    sovereign::memory::MemoryEngine* memory)
    : memory_(memory)
{
}

std::string MemoryBackedInterestGraph::NormalizeTopic(const std::string& topic)
{
    return Trim(topic);
}

bool MemoryBackedInterestGraph::IsInterestRecord(
    const sovereign::memory::MemoryRecord& record)
{
    return std::find(record.tags.begin(), record.tags.end(), "interest") != record.tags.end() ||
           std::find(record.tags.begin(), record.tags.end(), "graph") != record.tags.end();
}

InterestGraphEntry MemoryBackedInterestGraph::ParseRecord(
    const sovereign::memory::MemoryRecord& record)
{
    InterestGraphEntry value;
    if (record.body.empty())
        return value;

    const std::string prefix = "interest:";
    std::string interest_text = ReadValueAfterPrefix(record.body, prefix);
    if (interest_text.empty())
    {
        const auto topic_pos = record.body.find("|topic=");
        if (topic_pos != std::string::npos)
            interest_text = record.body.substr(topic_pos + 7);
    }

    std::string topic = interest_text;
    const auto score_separator = topic.find("|score=");
    if (score_separator != std::string::npos)
    {
        topic = Trim(topic.substr(0, score_separator));
    }
    const auto mentions_separator = topic.find("|mentions=");
    if (mentions_separator != std::string::npos)
    {
        topic = Trim(topic.substr(0, mentions_separator));
    }

    if (!topic.empty())
    {
        topic = Trim(topic);
        const auto score_pos = record.body.find("score=");
        const auto mentions_pos = record.body.find("mentions=");
        value.topic = topic;
        if (score_pos != std::string::npos)
        {
            const std::string score_value = record.body.substr(score_pos + 6);
            const auto end = score_value.find('|');
            const std::string score_text = end == std::string::npos ? score_value : score_value.substr(0, end);
            value.score = std::stod(score_text);
        }
        if (mentions_pos != std::string::npos)
        {
            const std::string mentions_value = record.body.substr(mentions_pos + 9);
            const auto end = mentions_value.find('|');
            const std::string mentions_text = end == std::string::npos ? mentions_value : mentions_value.substr(0, end);
            value.mentions = static_cast<std::uint32_t>(std::stoul(mentions_text));
        }
        value.last_seen_unix_seconds = record.created_unix_seconds;
    }
    return value;
}

void MemoryBackedInterestGraph::Observe(
    const std::string& topic,
    double weight,
    std::uint64_t now)
{
    const std::string normalized = NormalizeTopic(topic);
    if (normalized.empty())
    {
        return;
    }

    const std::uint64_t current_time = now == 0 ? NowUnixSeconds() : now;
    auto found = std::find_if(entries_.begin(), entries_.end(), [&](const InterestGraphEntry& item) {
        return item.topic == normalized;
    });
    if (found == entries_.end())
    {
        InterestGraphEntry entry;
        entry.topic = normalized;
        entry.score = std::max(0.0, weight);
        entry.mentions = 1;
        entry.last_seen_unix_seconds = current_time;
        entries_.push_back(entry);
    }
    else
    {
        found->score += std::max(0.0, weight);
        found->mentions += 1;
        found->last_seen_unix_seconds = current_time;
    }

    if (memory_ != nullptr)
    {
        sovereign::memory::MemoryRecord rec;
        rec.author = sovereign::PrincipalId{};
        rec.tags = {"cognitive", "interest", "graph"};
        rec.body = "interest:" + normalized + "|score=" + std::to_string(std::max(0.0, weight)) + "|mentions=1";
        rec.importance = std::clamp(std::max(0.0, weight), 0.0, 1.0);
        rec.created_unix_seconds = current_time;
        static_cast<void>(memory_->Append(rec));
    }
}

std::vector<InterestGraphEntry> MemoryBackedInterestGraph::TopTopics(
    std::size_t limit) const
{
    std::map<std::string, InterestGraphEntry> merged;
    for (const auto& entry : entries_)
    {
        merged[entry.topic] = entry;
    }
    if (memory_ != nullptr)
    {
        const auto all = memory_->ListAll();
        if (all.Ok())
        {
            for (const auto& record : all.value)
            {
                if (!IsInterestRecord(record))
                    continue;
                const auto parsed = ParseRecord(record);
                if (parsed.topic.empty())
                    continue;
                auto it = merged.find(parsed.topic);
                if (it == merged.end())
                {
                    merged[parsed.topic] = parsed;
                }
                else
                {
                    it->second.score += parsed.score;
                    it->second.mentions += parsed.mentions;
                    it->second.last_seen_unix_seconds = std::max(
                        it->second.last_seen_unix_seconds,
                        parsed.last_seen_unix_seconds);
                }
            }
        }
    }

    std::vector<InterestGraphEntry> items;
    items.reserve(merged.size());
    for (const auto& [topic, entry] : merged)
    {
        items.push_back(entry);
    }
    std::sort(items.begin(), items.end(), [](const InterestGraphEntry& lhs, const InterestGraphEntry& rhs) {
        if (lhs.score != rhs.score)
            return lhs.score > rhs.score;
        return lhs.last_seen_unix_seconds > rhs.last_seen_unix_seconds;
    });
    if (limit > 0 && items.size() > limit)
        items.resize(limit);
    return items;
}

bool MemoryBackedInterestGraph::HasTopic(const std::string& topic) const
{
    const std::string normalized = NormalizeTopic(topic);
    return std::any_of(entries_.begin(), entries_.end(), [&](const InterestGraphEntry& item) {
        return item.topic == normalized;
    });
}

CuriosityEngine::CuriosityEngine(sovereign::memory::MemoryEngine* memory)
    : memory_(memory)
    , interest_graph_(memory)
{
}

std::vector<std::string> CuriosityEngine::DefaultQuestions(
    const std::string& topic) const
{
    std::vector<std::string> questions;
    const std::string clean = NormalizeTopic(topic);

    if (clean.empty())
    {
        questions.push_back("What is the current state of the system?");
        questions.push_back("What evidence would change the current assessment?");
        questions.push_back("What constraints or risks matter most?");
        return questions;
    }

    questions.push_back("What is the current baseline for " + clean + "?");
    questions.push_back("What evidence or signals would change the answer for " + clean + "?");
    questions.push_back("What assumptions or constraints matter most for " + clean + "?");
    return questions;
}

void CuriosityEngine::QueueDiscovery(
    const std::string& topic,
    double priority)
{
    discovery_queue_.Enqueue(topic, priority, Now());
}

std::vector<DiscoveryQueueEntry> CuriosityEngine::DiscoveryQueueSnapshot() const
{
    return discovery_queue_.Snapshot();
}

std::vector<DiscoveryQueueEntry> CuriosityEngine::NextDiscoveryBatch(
    std::size_t max)
{
    return discovery_queue_.PopReady(Now(), max);
}

void CuriosityEngine::ObserveInterest(const std::string& topic, double weight)
{
    interest_graph_.Observe(topic, weight, Now());
}

std::vector<InterestGraphEntry> CuriosityEngine::InterestGraph(
    std::size_t limit) const
{
    return interest_graph_.TopTopics(limit);
}

Result<CuriosityProposal> CuriosityEngine::Propose(
    const PrincipalId& requester,
    const std::string& topic,
    const std::vector<std::string>& questions)
{
    Result<CuriosityProposal> result;
    std::string clean_topic = NormalizeTopic(topic);
    if (clean_topic.empty())
    {
        result.error = CognitiveError::InvalidArgument;
        return result;
    }

    CuriosityProposal p;
    p.id = NewId();
    p.requester = requester;
    p.topic = clean_topic;

    std::vector<std::string> generated = questions;
    generated.erase(
        std::remove_if(generated.begin(), generated.end(), [](const std::string& q) {
            return q.empty();
        }),
        generated.end());
    if (generated.empty())
    {
        generated = DefaultQuestions(clean_topic);
    }
    p.questions = std::move(generated);
    p.created_unix_seconds = Now();

    QueueDiscovery(clean_topic, 0.8);
    ObserveInterest(clean_topic, 1.0);

    if (memory_ != nullptr)
    {
        sovereign::memory::MemoryRecord rec;
        rec.author = requester;
        rec.tags = {"cognitive", "curiosity"};
        rec.body = "topic:" + p.topic;
        rec.importance = 0.8;
        static_cast<void>(memory_->Append(rec));
    }

    result.value = p;
    return result;
}

BoundedProbeReport CuriosityEngine::RunBoundedProbe(
    const PrincipalId& requester,
    const std::string& probe_action,
    const std::string& topic)
{
    BoundedProbeReport rep;
    rep.probe_action = probe_action;
    rep.topic = NormalizeTopic(topic);
    if (rep.topic.empty())
    {
        rep.reason = "empty_topic";
        return rep;
    }
    const bool safe_action = (probe_action == "observe" || probe_action == "inspect" ||
                              probe_action == "search" || probe_action == "plan" || probe_action == "chat");
    if (!safe_action)
    {
        rep.reason = "policy_denied_mutating_probe:" + probe_action;
        return rep;
    }
    auto prop = Propose(requester, rep.topic, {});
    if (!prop.Ok())
    {
        rep.reason = "propose_failed";
        return rep;
    }
    ObserveInterest(rep.topic, 1.0);
    QueueDiscovery(rep.topic, 0.7);
    rep.allowed = true;
    rep.questions = prop.value.questions;
    rep.reason = "bounded_probe_allowed";
    return rep;
}

} // namespace sovereign::cognitive
