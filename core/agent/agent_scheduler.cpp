#include "agent_scheduler.hpp"
#include <fstream>
#include <random>
namespace sovereign::agent {
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
JobId AgentScheduler::NewId(){JobId id{};std::random_device rd;for(auto& x:id.bytes)x=uint8_t(rd());return id;}
AgentError AgentScheduler::Open(const std::filesystem::path& d){
  std::lock_guard lock(mutex_); directory_=d; std::error_code ec;
  std::filesystem::create_directories(directory_,ec); if(ec)return AgentError::StorageFailure;
  open_=true; return Load();}
void AgentScheduler::Close() noexcept{std::lock_guard lock(mutex_); if(open_)(void)Save(); open_=false;}
AgentError AgentScheduler::Load()
{
  jobs_.clear();
  std::ifstream in(directory_ / "jobs.dat");
  std::string line;
  while (std::getline(in, line))
  {
    if (line.empty())
      continue;
    std::vector<std::string> parts;
    std::string cur;
    for (size_t i = 0; i < line.size(); ++i)
    {
      if (line[i] == '\' && i + 1 < line.size())
      {
        cur.push_back(line[i++]);
        cur.push_back(line[i]);
      }
      else if (line[i] == '|')
      {
        parts.push_back(cur);
        cur.clear();
      }
      else
      {
        cur.push_back(line[i]);
      }
    }
    parts.push_back(cur);
    if (parts.size() < 6)
      continue;
    ScheduledJob j;
    if (!Unhex(parts[0], j.id.bytes))
      continue;
    j.kind = static_cast<JobKind>(std::stoi(parts[1]));
    j.name = Unesc(parts[2]);
    j.interval_seconds = std::stoull(parts[3]);
    j.last_run_unix = std::stoull(parts[4]);
    j.enabled = std::stoi(parts[5]) != 0;
    jobs_.push_back(j);
  }
  return AgentError::Ok;
}
AgentError AgentScheduler::Save() const{
  std::ofstream out(directory_/"jobs.dat",std::ios::trunc); if(!out)return AgentError::StorageFailure;
  for(const auto& j:jobs_){
    out<<Hex(j.id.bytes)<<'|'<<int(j.kind)<<'|'<<Esc(j.name)<<'|'<<j.interval_seconds
       <<'|'<<j.last_run_unix<<'|'<<(j.enabled?1:0)<<'\n';} return AgentError::Ok;}
Result<JobId> AgentScheduler::Add(const ScheduledJob& job){
  std::lock_guard lock(mutex_); Result<JobId> r; if(!open_||job.name.empty()){r.error=AgentError::InvalidArgument;return r;}
  ScheduledJob j=job; bool nil=true; for(auto b:j.id.bytes)if(b){nil=false;break;} if(nil)j.id=NewId();
  jobs_.push_back(j); if(Save()!=AgentError::Ok){jobs_.pop_back();r.error=AgentError::StorageFailure;return r;}
  r.value=j.id; return r;}
Result<std::vector<ScheduledJob>> AgentScheduler::List() const{
  std::lock_guard lock(mutex_); Result<std::vector<ScheduledJob>> r; r.value=jobs_; return r;}
Result<void> AgentScheduler::MarkRun(const JobId& id, std::uint64_t when){
  std::lock_guard lock(mutex_); Result<void> r;
  for(auto& j:jobs_){ if(j.id.bytes==id.bytes){ j.last_run_unix=when; r.error=Save(); return r;}}
  r.error=AgentError::NotFound; return r;}
Result<std::vector<ScheduledJob>> AgentScheduler::Due(std::uint64_t now) const{
  std::lock_guard lock(mutex_); Result<std::vector<ScheduledJob>> r;
  for(const auto& j:jobs_){ if(!j.enabled)continue;
    if(j.last_run_unix==0 || now>=j.last_run_unix+j.interval_seconds) r.value.push_back(j);}
  return r;}
}
