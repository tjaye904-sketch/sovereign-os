#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace sovereign::agent {
struct JobId { std::array<std::uint8_t,16> bytes{}; };
enum class JobKind : std::uint8_t { BriefSnapshot = 1, MemoryDigest = 2, Custom = 3 };
struct ScheduledJob {
  JobId id{};
  JobKind kind{JobKind::BriefSnapshot};
  std::string name;
  std::uint64_t interval_seconds{86400}; // default daily
  std::uint64_t last_run_unix{0};
  bool enabled{true};
};
enum class AgentError : std::uint8_t { Ok=0, NotFound, InvalidArgument, StorageFailure };
template<typename T> struct Result {
  AgentError error{AgentError::Ok}; T value{};
  bool Ok() const noexcept { return error==AgentError::Ok; }
};
template<> struct Result<void> {
  AgentError error{AgentError::Ok};
  bool Ok() const noexcept { return error==AgentError::Ok; }
};
}
