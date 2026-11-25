
#include "archive_service.hpp"
#include <chrono>
#include <fstream>
#include <random>
namespace sovereign::archive {
namespace {
std::string Hex(const std::array<std::uint8_t,16>& b){
  static constexpr char k[]="0123456789abcdef"; std::string o(32,'0');
  for(size_t i=0;i<16;++i){o[i*2]=k[(b[i]>>4)&0xF];o[i*2+1]=k[b[i]&0xF];} return o;}
bool Unhex(const std::string& h, std::array<std::uint8_t,16>& b){
  if(h.size()!=32)return false;
  auto n=[](char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;};
  for(size_t i=0;i<16;++i){int hi=n(h[i*2]),lo=n(h[i*2+1]);if(hi<0||lo<0)return false;b[i]=uint8_t((hi<<4)|lo);} return true;}
std::string Esc(const std::string& s){std::string o;for(char c:s){if(c=='|'||c=='\\'||c=='\n'){o.push_back('\\');o.push_back(c=='\n'?'n':c);}else o.push_back(c);}return o;}
std::string Unesc(const std::string& s){std::string o;for(size_t i=0;i<s.size();++i){if(s[i]=='\\'&&i+1<s.size()){++i;o.push_back(s[i]=='n'?'\n':s[i]);}else o.push_back(s[i]);}return o;}
}
ArtifactId ArchiveService::NewId(){ArtifactId id{};std::random_device rd;for(auto& x:id.bytes)x=uint8_t(rd());return id;}
std::uint64_t ArchiveService::Now(){using namespace std::chrono;return uint64_t(duration_cast<seconds>(system_clock::now().time_since_epoch()).count());}
ArchiveError ArchiveService::Open(const std::filesystem::path& d){std::lock_guard lock(mutex_);directory_=d;std::error_code ec;std::filesystem::create_directories(directory_,ec);if(ec)return ArchiveError::StorageFailure;open_=true;return Load();}
void ArchiveService::Close() noexcept{std::lock_guard lock(mutex_);if(open_)(void)Save();open_=false;}
ArchiveError ArchiveService::Load(){artifacts_.clear();std::ifstream in(directory_/"artifacts.dat");std::string line;while(std::getline(in,line)){if(line.empty())continue;std::vector<std::string> p;std::string cur;for(size_t i=0;i<line.size();++i){if(line[i]=='\\'&&i+1<line.size()){cur.push_back(line[i++]);cur.push_back(line[i]);}else if(line[i]=='|'){p.push_back(cur);cur.clear();}else cur.push_back(line[i]);}p.push_back(cur);if(p.size()<5)continue;Artifact a;if(!Unhex(p[0],a.id.bytes))continue;Unhex(p[1],a.author.bytes);a.content_hash=Unesc(p[2]);a.label=Unesc(p[3]);a.created_unix_seconds=std::stoull(p[4]);artifacts_.push_back(a);}return ArchiveError::Ok;}
ArchiveError ArchiveService::Save() const{std::ofstream out(directory_/"artifacts.dat",std::ios::trunc);if(!out)return ArchiveError::StorageFailure;for(const auto& a:artifacts_){out<<Hex(a.id.bytes)<<'|'<<Hex(a.author.bytes)<<'|'<<Esc(a.content_hash)<<'|'<<Esc(a.label)<<'|'<<a.created_unix_seconds<<'\n';}return ArchiveError::Ok;}
Result<ArtifactId> ArchiveService::Store(const Artifact& artifact){std::lock_guard lock(mutex_);Result<ArtifactId> r;if(!open_||artifact.content_hash.empty()){r.error=ArchiveError::InvalidArgument;return r;}Artifact a=artifact;bool nil=true;for(auto b:a.id.bytes)if(b){nil=false;break;}if(nil)a.id=NewId();if(!a.created_unix_seconds)a.created_unix_seconds=Now();artifacts_.push_back(a);if(Save()!=ArchiveError::Ok){artifacts_.pop_back();r.error=ArchiveError::StorageFailure;return r;}r.value=a.id;return r;}
Result<Artifact> ArchiveService::Get(const ArtifactId& id) const{std::lock_guard lock(mutex_);Result<Artifact> r;for(const auto& a:artifacts_)if(a.id==id){r.value=a;return r;}r.error=ArchiveError::NotFound;return r;}
Result<std::vector<Artifact>> ArchiveService::List() const{std::lock_guard lock(mutex_);Result<std::vector<Artifact>> r;r.value=artifacts_;return r;}
} // namespace

namespace {
std::uint64_t HashBytesFnv1a(const std::filesystem::path& dir, std::size_t& files, std::uint64_t& bytes) {
  std::uint64_t h = 14695981039346656037ULL;
  std::error_code ec;
  if (!std::filesystem::exists(dir, ec)) return h;
  std::vector<std::filesystem::path> sorted;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(dir, ec)) {
    if (entry.is_regular_file(ec) && entry.path().filename() != "MANIFEST.dat") sorted.push_back(entry.path());
  }
  std::sort(sorted.begin(), sorted.end());
  for (const auto& p : sorted) {
    const auto rel = std::filesystem::relative(p, dir, ec).generic_string();
    for (unsigned char c : rel) { h ^= c; h *= 1099511628211ULL; }
    std::ifstream in(p, std::ios::binary);
    char buf[4096];
    while (in.read(buf, sizeof(buf)) || in.gcount() > 0) {
      const auto n = static_cast<std::size_t>(in.gcount());
      bytes += n;
      for (std::size_t i = 0; i < n; ++i) { h ^= static_cast<unsigned char>(buf[i]); h *= 1099511628211ULL; }
    }
    ++files;
  }
  return h;
}
std::string Hex64(std::uint64_t v) {
  char out[17]{};
  std::snprintf(out, sizeof(out), "%016llx", static_cast<unsigned long long>(v));
  return out;
}
} // namespace

namespace sovereign::archive {
CoreBackupReport ArchiveService::CreateCoreSnapshot(const std::filesystem::path& data_root, const std::filesystem::path& snapshot_dir) {
  CoreBackupReport rep;
  rep.snapshot_dir = snapshot_dir;
  std::error_code ec;
  if (!std::filesystem::exists(data_root, ec)) { rep.detail = "data_root_missing"; return rep; }
  std::filesystem::remove_all(snapshot_dir, ec);
  std::filesystem::create_directories(snapshot_dir / "state", ec);
  for (const auto& entry : std::filesystem::directory_iterator(data_root, ec)) {
    if (entry.is_directory(ec)) {
      std::filesystem::copy(entry.path(), snapshot_dir / "state" / entry.path().filename(),
        std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing, ec);
    }
  }
  const auto h = HashBytesFnv1a(snapshot_dir / "state", rep.file_count, rep.total_bytes);
  rep.checksum_hex = Hex64(h);
  std::ofstream m(snapshot_dir / "MANIFEST.dat", std::ios::trunc);
  m << "checksum=" << rep.checksum_hex << "\nfiles=" << rep.file_count << "\nbytes=" << rep.total_bytes << "\n";
  rep.ok = m.good();
  rep.detail = rep.ok ? "snapshot_created" : "manifest_write_failed";
  return rep;
}
CoreBackupReport ArchiveService::VerifyCoreSnapshot(const std::filesystem::path& snapshot_dir) {
  CoreBackupReport rep;
  rep.snapshot_dir = snapshot_dir;
  std::ifstream m(snapshot_dir / "MANIFEST.dat");
  if (!m.is_open()) { rep.detail = "manifest_missing"; return rep; }
  std::string line, expected;
  while (std::getline(m, line)) if (line.rfind("checksum=", 0) == 0) expected = line.substr(9);
  const auto actual = Hex64(HashBytesFnv1a(snapshot_dir / "state", rep.file_count, rep.total_bytes));
  rep.checksum_hex = actual;
  rep.ok = !expected.empty() && actual == expected;
  rep.detail = rep.ok ? "snapshot_verified" : "checksum_mismatch";
  return rep;
}
CoreBackupReport ArchiveService::RestoreCoreSnapshot(const std::filesystem::path& snapshot_dir, const std::filesystem::path& target_data_root) {
  auto ver = VerifyCoreSnapshot(snapshot_dir);
  if (!ver.ok) return ver;
  std::error_code ec;
  std::filesystem::create_directories(target_data_root, ec);
  for (const auto& entry : std::filesystem::directory_iterator(snapshot_dir / "state", ec)) {
    if (entry.is_directory(ec)) {
      const auto dst = target_data_root / entry.path().filename();
      std::filesystem::remove_all(dst, ec);
      std::filesystem::copy(entry.path(), dst,
        std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing, ec);
    }
  }
  ver.detail = "snapshot_restored";
  return ver;
}
} // namespace sovereign::archive
