#pragma once
#include "agent_types.hpp"
#include <filesystem>
#include <mutex>
#include <vector>
namespace sovereign::agent {
class AgentScheduler {
public:
  AgentError Open(const std::filesystem::path& directory);
  void Close() noexcept;
  Result<JobId> Add(const ScheduledJob& job);
  Result<std::vector<ScheduledJob>> List() const;
  Result<void> MarkRun(const JobId& id, std::uint64_t when_unix);
  Result<std::vector<ScheduledJob>> Due(std::uint64_t now_unix) const;
private:
  AgentError Load();
  AgentError Save() const;
  static JobId NewId();
  mutable std::mutex mutex_;
  bool open_{false};
  std::filesystem::path directory_;
  std::vector<ScheduledJob> jobs_;
};
}
