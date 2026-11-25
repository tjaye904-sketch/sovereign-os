#pragma once
// Append-only archive of immutable artifacts (hashes + metadata).
#include "principal_id.hpp"
#include <algorithm>
#include <cstdio>
#include <fstream>
#include <array>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

namespace sovereign::archive
{
using ::sovereign::PrincipalId;
struct ArtifactId { std::array<std::uint8_t, 16> bytes{}; };
inline bool operator==(const ArtifactId& a, const ArtifactId& b) noexcept {
  return a.bytes == b.bytes;
}
struct Artifact {
  ArtifactId id{};
  PrincipalId author{};
  std::string content_hash;
  std::string label;
  std::uint64_t created_unix_seconds{0};
};
enum class ArchiveError : std::uint8_t {
  Ok=0, NotFound, InvalidArgument, StorageFailure
};
template<typename T> struct Result {
  ArchiveError error{ArchiveError::Ok}; T value{};
  bool Ok() const noexcept { return error==ArchiveError::Ok; }
};
template<> struct Result<void> {
  ArchiveError error{ArchiveError::Ok};
  bool Ok() const noexcept { return error==ArchiveError::Ok; }
};
struct CoreBackupReport {
  bool ok{false};
  std::filesystem::path snapshot_dir;
  std::size_t file_count{0};
  std::uint64_t total_bytes{0};
  std::string checksum_hex;
  std::string detail;
};

class ArchiveService {
public:
  ArchiveError Open(const std::filesystem::path& directory);
  void Close() noexcept;
  Result<ArtifactId> Store(const Artifact& artifact);
  Result<Artifact> Get(const ArtifactId& id) const;
  Result<std::vector<Artifact>> List() const;
  static CoreBackupReport CreateCoreSnapshot(const std::filesystem::path& data_root, const std::filesystem::path& snapshot_dir);
  static CoreBackupReport VerifyCoreSnapshot(const std::filesystem::path& snapshot_dir);
  static CoreBackupReport RestoreCoreSnapshot(const std::filesystem::path& snapshot_dir, const std::filesystem::path& target_data_root);
private:
  ArchiveError Load();
  ArchiveError Save() const;
  static ArtifactId NewId();
  static std::uint64_t Now();
  mutable std::mutex mutex_;
  bool open_{false};
  std::filesystem::path directory_;
  std::vector<Artifact> artifacts_;
};
} // namespace sovereign::archive
