#pragma once

#include "cognitive_types.hpp"
#include "memory_engine.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::cognitive
{

struct DiscoveryQueueEntry
{
    std::string topic;
    double priority{0.0};
    std::uint64_t created_unix_seconds{0};
    std::uint64_t next_run_unix_seconds{0};
    std::uint32_t attempts{0};
};

struct InterestGraphEntry
{
    std::string topic;
    double score{0.0};
    std::uint32_t mentions{0};
    std::uint64_t last_seen_unix_seconds{0};
};

class BackgroundDiscoveryQueue
{
public:
    void Enqueue(const std::string& topic, double priority = 0.5, std::uint64_t now = 0);
    std::vector<DiscoveryQueueEntry> Snapshot() const;
    std::vector<DiscoveryQueueEntry> PopReady(std::uint64_t now, std::size_t max = 10);
    bool Empty() const noexcept;

private:
    static std::string NormalizeTopic(const std::string& topic);

    std::vector<DiscoveryQueueEntry> entries_;
};

class MemoryBackedInterestGraph
{
public:
    explicit MemoryBackedInterestGraph(sovereign::memory::MemoryEngine* memory = nullptr);

    void Observe(const std::string& topic, double weight = 1.0, std::uint64_t now = 0);
    std::vector<InterestGraphEntry> TopTopics(std::size_t limit = 10) const;
    bool HasTopic(const std::string& topic) const;

private:
    static std::string NormalizeTopic(const std::string& topic);
    static bool IsInterestRecord(const sovereign::memory::MemoryRecord& record);
    static InterestGraphEntry ParseRecord(const sovereign::memory::MemoryRecord& record);

    sovereign::memory::MemoryEngine* memory_;
    std::vector<InterestGraphEntry> entries_;
};

class CuriosityEngine
{
public:
    explicit CuriosityEngine(sovereign::memory::MemoryEngine* memory = nullptr);

    Result<CuriosityProposal> Propose(
        const PrincipalId& requester,
        const std::string& topic,
        const std::vector<std::string>& questions);

    std::vector<std::string> DefaultQuestions(const std::string& topic) const;

    void QueueDiscovery(const std::string& topic, double priority = 0.5);
    std::vector<DiscoveryQueueEntry> DiscoveryQueueSnapshot() const;
    std::vector<DiscoveryQueueEntry> NextDiscoveryBatch(std::size_t max = 10);

    void ObserveInterest(const std::string& topic, double weight = 1.0);
    std::vector<InterestGraphEntry> InterestGraph(std::size_t limit = 10) const;

private:
    static AnalysisId NewId();
    static std::uint64_t Now();
    static std::string NormalizeTopic(const std::string& topic);

    sovereign::memory::MemoryEngine* memory_;
    BackgroundDiscoveryQueue discovery_queue_;
    MemoryBackedInterestGraph interest_graph_;
};

} // namespace sovereign::cognitive
