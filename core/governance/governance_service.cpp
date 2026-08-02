#include "governance_service.hpp"

#include <chrono>
#include <fstream>
#include <random>

namespace sovereign::governance
{
namespace
{

std::string Hex(const std::array<std::uint8_t, 16>& b)
{
    static constexpr char k[] = "0123456789abcdef";
    std::string o(32, '0');
    for (std::size_t i = 0; i < 16; ++i)
    {
        o[i * 2] = k[(b[i] >> 4) & 0xF];
        o[i * 2 + 1] = k[b[i] & 0xF];
    }
    return o;
}

bool Unhex(const std::string& h, std::array<std::uint8_t, 16>& b)
{
    if (h.size() != 32)
        return false;
    auto n = [](char c) {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < 16; ++i)
    {
        int hi = n(h[i * 2]), lo = n(h[i * 2 + 1]);
        if (hi < 0 || lo < 0)
            return false;
        b[i] = static_cast<std::uint8_t>((hi << 4) | lo);
    }
    return true;
}

std::string Esc(const std::string& s)
{
    std::string o;
    for (char c : s)
    {
        if (c == '|' || c == '\\' || c == '\n')
        {
            o.push_back('\\');
            o.push_back(c == '\n' ? 'n' : c);
        }
        else
            o.push_back(c);
    }
    return o;
}

std::string Unesc(const std::string& s)
{
    std::string o;
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '\\' && i + 1 < s.size())
        {
            ++i;
            o.push_back(s[i] == 'n' ? '\n' : s[i]);
        }
        else
            o.push_back(s[i]);
    }
    return o;
}

} // namespace

DecisionId GovernanceService::NewId()
{
    DecisionId id{};
    std::random_device rd;
    for (auto& x : id.bytes)
        x = static_cast<std::uint8_t>(rd());
    return id;
}

std::uint64_t GovernanceService::Now()
{
    using namespace std::chrono;
    return static_cast<std::uint64_t>(
        duration_cast<seconds>(system_clock::now().time_since_epoch()).count());
}

GovernanceError GovernanceService::Open(const std::filesystem::path& directory)
{
    std::lock_guard lock(mutex_);
    directory_ = directory;
    std::error_code ec;
    std::filesystem::create_directories(directory_, ec);
    if (ec)
        return GovernanceError::StorageFailure;
    open_ = true;
    const auto e1 = Load();
    if (e1 != GovernanceError::Ok)
        return e1;
    return LoadRules();
}

void GovernanceService::Close() noexcept
{
    std::lock_guard lock(mutex_);
    if (open_)
    {
        static_cast<void>(Save());
        static_cast<void>(SaveRules());
    }
    open_ = false;
}

GovernanceError GovernanceService::Load()
{
    decisions_.clear();
    std::ifstream in(directory_ / "decisions.dat");
    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty())
            continue;
        std::vector<std::string> p;
        std::string cur;
        for (std::size_t i = 0; i < line.size(); ++i)
        {
            if (line[i] == '\\' && i + 1 < line.size())
            {
                cur.push_back(line[i++]);
                cur.push_back(line[i]);
            }
            else if (line[i] == '|')
            {
                p.push_back(cur);
                cur.clear();
            }
            else
                cur.push_back(line[i]);
        }
        p.push_back(cur);
        if (p.size() < 9)
            continue;
        Decision d;
        if (!Unhex(p[0], d.id.bytes))
            continue;
        Unhex(p[1], d.requester.bytes);
        d.action = Unesc(p[2]);
        d.summary = Unesc(p[3]);
        d.evidence_ref = Unesc(p[4]);
        d.status = static_cast<DecisionStatus>(std::stoi(p[5]));
        Unhex(p[6], d.resolved_by.bytes);
        d.resolution_reason = Unesc(p[7]);
        d.created_unix_seconds = std::stoull(p[8]);
        if (p.size() >= 10)
            d.resolved_unix_seconds = std::stoull(p[9]);
        if (p.size() >= 11)
            d.is_override = std::stoi(p[10]) != 0;
        decisions_.push_back(d);
    }
    return GovernanceError::Ok;
}

GovernanceError GovernanceService::Save() const
{
    std::ofstream out(directory_ / "decisions.dat", std::ios::trunc);
    if (!out)
        return GovernanceError::StorageFailure;
    for (const auto& d : decisions_)
    {
        out << Hex(d.id.bytes) << '|' << Hex(d.requester.bytes) << '|'
            << Esc(d.action) << '|' << Esc(d.summary) << '|'
            << Esc(d.evidence_ref) << '|' << static_cast<int>(d.status) << '|'
            << Hex(d.resolved_by.bytes) << '|' << Esc(d.resolution_reason)
            << '|' << d.created_unix_seconds << '|' << d.resolved_unix_seconds
            << '|' << (d.is_override ? 1 : 0) << '\n';
    }
    return GovernanceError::Ok;
}

GovernanceError GovernanceService::LoadRules()
{
    rules_.clear();
    std::ifstream in(directory_ / "rules.dat");
    std::string line;
    while (std::getline(in, line))
    {
        if (line.empty())
            continue;
        std::vector<std::string> p;
        std::string cur;
        for (std::size_t i = 0; i < line.size(); ++i)
        {
            if (line[i] == '|')
            {
                p.push_back(cur);
                cur.clear();
            }
            else
                cur.push_back(line[i]);
        }
        p.push_back(cur);
        if (p.size() < 4)
            continue;
        GovernanceRule r;
        r.action = Unesc(p[0]);
        r.allow_auto_approval = std::stoi(p[1]) != 0;
        r.minimum_auto_approve_role =
            static_cast<GovernanceRole>(std::stoi(p[2]));
        r.minimum_approval_authority_role =
            static_cast<GovernanceRole>(std::stoi(p[3]));
        rules_[r.action] = r;
    }
    return GovernanceError::Ok;
}

GovernanceError GovernanceService::SaveRules() const
{
    std::ofstream out(directory_ / "rules.dat", std::ios::trunc);
    if (!out)
        return GovernanceError::StorageFailure;
    for (const auto& [k, r] : rules_)
    {
        static_cast<void>(k);
        out << Esc(r.action) << '|' << (r.allow_auto_approval ? 1 : 0) << '|'
            << static_cast<int>(r.minimum_auto_approve_role) << '|'
            << static_cast<int>(r.minimum_approval_authority_role) << '\n';
    }
    return GovernanceError::Ok;
}

void GovernanceService::SetRule(GovernanceRule rule)
{
    std::lock_guard lock(mutex_);
    rules_[rule.action] = std::move(rule);
    static_cast<void>(SaveRules());
}

GovernanceRule GovernanceService::RuleFor(const std::string& action) const
{
    std::lock_guard lock(mutex_);
    auto it = rules_.find(action);
    if (it != rules_.end())
        return it->second;
    return DefaultGovernanceRule(action);
}

Result<DecisionId> GovernanceService::RequestDecision(
    const PrincipalId& requester,
    GovernanceRole requester_role,
    const std::string& action,
    const std::string& summary,
    const std::string& evidence_ref)
{
    std::lock_guard lock(mutex_);
    Result<DecisionId> result;
    if (!open_ || action.empty())
    {
        result.error = GovernanceError::InvalidArgument;
        return result;
    }
    Decision d;
    d.id = NewId();
    d.requester = requester;
    d.action = action;
    d.summary = summary;
    d.evidence_ref = evidence_ref;
    d.created_unix_seconds = Now();

    GovernanceRule rule = DefaultGovernanceRule(action);
    auto it = rules_.find(action);
    if (it != rules_.end())
        rule = it->second;

    if (rule.allow_auto_approval &&
        AtLeast(requester_role, rule.minimum_auto_approve_role))
    {
        d.status = DecisionStatus::Approved;
        d.resolved_by = requester;
        d.resolution_reason = "auto-approved by rule";
        d.resolved_unix_seconds = d.created_unix_seconds;
    }
    else
    {
        d.status = DecisionStatus::Pending;
    }

    decisions_.push_back(d);
    if (Save() != GovernanceError::Ok)
    {
        decisions_.pop_back();
        result.error = GovernanceError::StorageFailure;
        return result;
    }
    result.value = d.id;
    return result;
}

Result<DecisionId> GovernanceService::RequestDecision(
    const PrincipalId& requester,
    const std::string& action,
    const std::string& summary,
    const std::string& evidence_ref)
{
    return RequestDecision(
        requester, GovernanceRole::Operator, action, summary, evidence_ref);
}

Result<std::vector<Decision>> GovernanceService::ListPending() const
{
    std::lock_guard lock(mutex_);
    Result<std::vector<Decision>> result;
    for (const auto& d : decisions_)
    {
        if (d.status == DecisionStatus::Pending)
            result.value.push_back(d);
    }
    return result;
}

Result<Decision> GovernanceService::Get(const DecisionId& id) const
{
    std::lock_guard lock(mutex_);
    Result<Decision> result;
    for (const auto& d : decisions_)
    {
        if (d.id == id)
        {
            result.value = d;
            return result;
        }
    }
    result.error = GovernanceError::NotFound;
    return result;
}

Result<void> GovernanceService::Approve(
    const PrincipalId& actor,
    GovernanceRole actor_role,
    const DecisionId& id,
    const std::string& reason)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& d : decisions_)
    {
        if (d.id != id)
            continue;
        if (d.status != DecisionStatus::Pending)
        {
            result.error = GovernanceError::InvalidState;
            return result;
        }
        GovernanceRule rule = DefaultGovernanceRule(d.action);
        auto it = rules_.find(d.action);
        if (it != rules_.end())
            rule = it->second;
        if (!AtLeast(actor_role, rule.minimum_approval_authority_role))
        {
            result.error = GovernanceError::Unauthorized;
            return result;
        }
        d.status = DecisionStatus::Approved;
        d.resolved_by = actor;
        d.resolution_reason = reason;
        d.resolved_unix_seconds = Now();
        d.is_override = false;
        result.error = Save();
        return result;
    }
    result.error = GovernanceError::NotFound;
    return result;
}

Result<void> GovernanceService::Reject(
    const PrincipalId& actor,
    GovernanceRole actor_role,
    const DecisionId& id,
    const std::string& reason)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    for (auto& d : decisions_)
    {
        if (d.id != id)
            continue;
        if (d.status != DecisionStatus::Pending)
        {
            result.error = GovernanceError::InvalidState;
            return result;
        }
        GovernanceRule rule = DefaultGovernanceRule(d.action);
        auto it = rules_.find(d.action);
        if (it != rules_.end())
            rule = it->second;
        if (!AtLeast(actor_role, rule.minimum_approval_authority_role))
        {
            result.error = GovernanceError::Unauthorized;
            return result;
        }
        d.status = DecisionStatus::Rejected;
        d.resolved_by = actor;
        d.resolution_reason = reason;
        d.resolved_unix_seconds = Now();
        result.error = Save();
        return result;
    }
    result.error = GovernanceError::NotFound;
    return result;
}

Result<void> GovernanceService::Approve(
    const PrincipalId& owner,
    bool owner_is_owner_role,
    const DecisionId& id,
    const std::string& reason)
{
    return Approve(
        owner,
        owner_is_owner_role ? GovernanceRole::Owner : GovernanceRole::Guest,
        id,
        reason);
}

Result<void> GovernanceService::Reject(
    const PrincipalId& owner,
    bool owner_is_owner_role,
    const DecisionId& id,
    const std::string& reason)
{
    return Reject(
        owner,
        owner_is_owner_role ? GovernanceRole::Owner : GovernanceRole::Guest,
        id,
        reason);
}

Result<void> GovernanceService::Override(
    const PrincipalId& owner,
    GovernanceRole actor_role,
    const DecisionId& id,
    const std::string& reason)
{
    std::lock_guard lock(mutex_);
    Result<void> result;
    if (actor_role != GovernanceRole::Owner)
    {
        result.error = GovernanceError::Unauthorized;
        return result;
    }
    for (auto& d : decisions_)
    {
        if (d.id != id)
            continue;
        d.status = DecisionStatus::Approved;
        d.resolved_by = owner;
        d.resolution_reason = reason;
        d.resolved_unix_seconds = Now();
        d.is_override = true;
        result.error = Save();
        return result;
    }
    result.error = GovernanceError::NotFound;
    return result;
}

} // namespace sovereign::governance
