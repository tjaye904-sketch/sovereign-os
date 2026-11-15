#include "purpose_runtime.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <fstream>
#include <sstream>
#include <utility>

namespace sovereign::product
{
namespace
{
std::string TrimView(std::string value)
{
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos)
        return {};
    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

std::string StripQuotes(const std::string& value)
{
    std::string out = value;
    while (!out.empty() && (out.front() == '"' || out.front() == '\''))
    {
        out.erase(out.begin());
    }
    while (!out.empty() && (out.back() == '"' || out.back() == '\''))
    {
        out.pop_back();
    }
    return out;
}

std::vector<std::string> SplitList(const std::string& value)
{
    std::vector<std::string> items;
    std::stringstream ss(value);
    std::string part;
    while (std::getline(ss, part, ','))
    {
        auto cleaned = StripQuotes(TrimView(part));
        if (!cleaned.empty())
        {
            items.push_back(cleaned);
        }
    }
    return items;
}

std::string EscapeField(const std::string& input)
{
    std::string out;
    out.reserve(input.size());
    for (char ch : input)
    {
        switch (ch)
        {
            case '\\': out += "\\\\"; break;
            case '|': out += "\\|"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            default: out += ch; break;
        }
    }
    return out;
}

std::string UnescapeField(const std::string& input)
{
    std::string out;
    out.reserve(input.size());
    for (std::size_t i = 0; i < input.size(); ++i)
    {
        if (input[i] == '\\' && i + 1 < input.size())
        {
            const char next = input[i + 1];
            if (next == 'n')
            {
                out += '\n';
            }
            else if (next == 'r')
            {
                out += '\r';
            }
            else if (next == '|')
            {
                out += '|';
            }
            else if (next == '\\')
            {
                out += '\\';
            }
            else
            {
                out += next;
            }
            ++i;
            continue;
        }
        out += input[i];
    }
    return out;
}

std::vector<std::string> SplitEscapedFields(const std::string& value)
{
    std::vector<std::string> fields;
    std::string current;
    bool escaped = false;
    for (char ch : value)
    {
        if (escaped)
        {
            current += ch;
            escaped = false;
            continue;
        }
        if (ch == '\\')
        {
            escaped = true;
            current += ch;
            continue;
        }
        if (ch == '|')
        {
            fields.push_back(current);
            current.clear();
            continue;
        }
        current += ch;
    }
    fields.push_back(current);
    return fields;
}

} // namespace

PurposeRuntime::PurposeRuntime()
{
    PurposePack research;
    research.id = "research.web_collect";
    research.purpose_id = "research.web_collect";
    research.description = "Collect text from allowlisted docs and reference pages.";
    research.risk = PurposeRisk::Low;
    research.require_headed = false;
    research.allowed_hosts = {"docs.python.org", "manpages.ubuntu.com", "wikipedia.org"};
    research.allowed_apps = {"chromium", "firefox", "xdg-open"};
    research.allowed_tools = {
        "browser_open",
        "browser.open",
        "browser_snapshot",
        "browser.snapshot",
        "browser_screenshot",
        "desktop_snapshot",
        "desktop.snapshot",
        "app_open",
        "app.launch",
        "os_notify",
        "os.notify"};
    RegisterPack(research);

    PurposePack system_monitor;
    system_monitor.id = "ops.system_monitor";
    system_monitor.purpose_id = "ops.system_monitor";
    system_monitor.description = "Check the local system monitor and surface safe status.";
    system_monitor.risk = PurposeRisk::Low;
    system_monitor.require_headed = false;
    system_monitor.allowed_apps = {"gnome-system-monitor", "system-monitor"};
    system_monitor.allowed_tools = {"desktop_list_windows", "desktop.list_windows", "os_notify", "os.notify"};
    RegisterPack(system_monitor);

    PurposePack health_probe;
    health_probe.id = "ops.health_probe";
    health_probe.purpose_id = "ops.health_probe";
    health_probe.description = "Validate that the local host is healthy and report it safely.";
    health_probe.risk = PurposeRisk::Low;
    health_probe.require_headed = false;
    health_probe.allowed_tools = {"os_notify", "os.notify", "desktop_snapshot", "desktop.snapshot"};
    RegisterPack(health_probe);
}

void PurposeRuntime::LoadFromDirectory(const std::filesystem::path& directory)
{
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory))
    {
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }
        const auto ext = entry.path().extension().string();
        if (ext == ".yaml" || ext == ".yml")
        {
            LoadFromFile(entry.path());
        }
    }
}

void PurposeRuntime::LoadFromFile(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input.is_open())
    {
        return;
    }

    PurposePack pack;
    std::string line;
    std::string current_list;
    PurposeStep* current_step = nullptr;

    while (std::getline(input, line))
    {
        const auto trimmed = TrimView(line);
        if (trimmed.empty() || trimmed.rfind("#", 0) == 0)
        {
            continue;
        }

        if (trimmed.rfind("- ", 0) == 0)
        {
            if (current_list == "allowed_hosts" || current_list == "allowed_apps" || current_list == "allowed_tools")
            {
                const auto item = NormalizeScalar(TrimView(trimmed.substr(2)));
                if (!item.empty())
                {
                    if (current_list == "allowed_hosts")
                        pack.allowed_hosts.push_back(item);
                    else if (current_list == "allowed_apps")
                        pack.allowed_apps.push_back(item);
                    else
                        pack.allowed_tools.push_back(item);
                }
                continue;
            }

            if (current_list == "steps")
            {
                const auto content = TrimView(trimmed.substr(2));
                if (content.rfind("action:", 0) == 0)
                {
                    PurposeStep step;
                    step.action = NormalizeScalar(TrimView(content.substr(7)));
                    pack.steps.push_back(step);
                    current_step = &pack.steps.back();
                }
                else if (current_step != nullptr && content.rfind("args:", 0) == 0)
                {
                    current_step->detail = TrimView(content.substr(5));
                }
            }
            continue;
        }

        const auto colon = trimmed.find(':');
        if (colon == std::string::npos)
        {
            continue;
        }

        std::string key = NormalizeScalar(TrimView(trimmed.substr(0, colon)));
        std::string value = NormalizeScalar(TrimView(trimmed.substr(colon + 1)));
        current_list.clear();

        if (key == "id" || key == "purpose_id")
        {
            pack.id = value;
            pack.purpose_id = value;
        }
        else if (key == "description")
        {
            pack.description = value;
        }
        else if (key == "risk")
        {
            pack.risk = ParseRisk(value);
        }
        else if (key == "require_headed")
        {
            pack.require_headed = ParseBool(value);
        }
        else if (key == "allowed_hosts")
        {
            current_list = "allowed_hosts";
            for (const auto& item : SplitList(value))
            {
                if (!item.empty())
                    pack.allowed_hosts.push_back(item);
            }
        }
        else if (key == "allowed_apps")
        {
            current_list = "allowed_apps";
            for (const auto& item : SplitList(value))
            {
                if (!item.empty())
                    pack.allowed_apps.push_back(item);
            }
        }
        else if (key == "allowed_tools")
        {
            current_list = "allowed_tools";
            for (const auto& item : SplitList(value))
            {
                if (!item.empty())
                    pack.allowed_tools.push_back(item);
            }
        }
        else if (key == "steps")
        {
            current_list = "steps";
        }
    }

    if (!pack.id.empty())
    {
        RegisterPack(pack);
    }
}

bool PurposeRuntime::RegisterPack(PurposePack pack)
{
    if (pack.id.empty())
    {
        return false;
    }

    if (pack.purpose_id.empty())
    {
        pack.purpose_id = pack.id;
    }

    for (auto& existing : packs_)
    {
        if (existing.id == pack.id || existing.purpose_id == pack.id || existing.id == pack.purpose_id)
        {
            existing = std::move(pack);
            return true;
        }
    }

    packs_.push_back(std::move(pack));
    return true;
}

const std::vector<PurposePack>& PurposeRuntime::BuiltinPacks() const noexcept
{
    return packs_;
}

std::optional<PurposePack> PurposeRuntime::Find(const std::string& purpose_id) const
{
    const auto needle = ToLower(Trim(purpose_id));
    for (const auto& p : packs_)
    {
        if (ToLower(p.id) == needle || ToLower(p.purpose_id) == needle)
        {
            return p;
        }
    }
    return std::nullopt;
}

std::string PurposeRuntime::CurrentPurposeId() const noexcept
{
    return active_purpose_id_;
}

bool PurposeRuntime::SaveSession(const std::filesystem::path& path) const
{
    if (path.empty())
    {
        return false;
    }

    const auto parent = path.parent_path();
    if (!parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec)
        {
            return false;
        }
    }

    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open())
    {
        return false;
    }

    out << "purpose_id=" << EscapeField(active_purpose_id_) << '\n';
    out << "count=" << action_log_.size() << '\n';
    for (const auto& entry : action_log_)
    {
        out << "entry="
            << EscapeField(entry.timestamp) << '|'
            << EscapeField(entry.purpose_id) << '|'
            << EscapeField(entry.action) << '|'
            << EscapeField(entry.target) << '|'
            << EscapeField(entry.status) << '|'
            << EscapeField(entry.detail)
            << '\n';
    }
    return out.good();
}

bool PurposeRuntime::LoadSession(const std::filesystem::path& path)
{
    if (path.empty())
    {
        return false;
    }

    std::ifstream input(path);
    if (!input.is_open())
    {
        return false;
    }

    std::vector<PurposeActionLog> loaded;
    std::string purpose_id;
    std::string line;
    while (std::getline(input, line))
    {
        if (line.empty())
        {
            continue;
        }

        if (line.rfind("purpose_id=", 0) == 0)
        {
            purpose_id = UnescapeField(line.substr(11));
            continue;
        }

        if (line.rfind("count=", 0) == 0)
        {
            continue;
        }

        if (line.rfind("entry=", 0) == 0)
        {
            const auto payload = line.substr(6);
            const auto parts = SplitEscapedFields(payload);
            if (parts.size() < 6)
            {
                continue;
            }

            PurposeActionLog entry;
            entry.timestamp = UnescapeField(parts[0]);
            entry.purpose_id = UnescapeField(parts[1]);
            entry.action = UnescapeField(parts[2]);
            entry.target = UnescapeField(parts[3]);
            entry.status = UnescapeField(parts[4]);
            entry.detail = UnescapeField(parts[5]);
            loaded.push_back(entry);
        }
    }

    if (!purpose_id.empty() || !loaded.empty())
    {
        active_purpose_id_ = purpose_id;
        action_log_ = std::move(loaded);
        return true;
    }
    return false;
}

bool PurposeRuntime::StartPurpose(const std::string& purpose_id, const std::string& summary)
{
    const auto pack = Find(purpose_id);
    if (!pack)
    {
        return false;
    }
    active_purpose_id_ = pack->id;
    LogAction(pack->id, "purpose_start", {}, summary.empty() ? "purpose session started" : summary, "started");
    return true;
}

void PurposeRuntime::LogAction(
    const std::string& purpose_id,
    const std::string& action,
    const std::string& target,
    const std::string& detail,
    const std::string& status) const
{
    if (purpose_id.empty())
    {
        return;
    }

    const auto now = std::chrono::system_clock::now();
    const auto secs = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    std::stringstream stamp;
    stamp << secs;

    PurposeActionLog entry;
    entry.purpose_id = purpose_id;
    entry.action = action;
    entry.target = target;
    entry.status = status;
    entry.detail = detail;
    entry.timestamp = stamp.str();
    action_log_.push_back(entry);
}

std::vector<PurposeActionLog> PurposeRuntime::ActionLog() const
{
    return action_log_;
}

PurposeExecutionResult PurposeRuntime::Run(
    const std::string& purpose_id,
    const std::string& target,
    const std::string& summary) const
{
    const auto pack = Find(purpose_id);
    if (!pack)
    {
        return PurposeExecutionResult{
            .purpose_id = purpose_id,
            .allowed = false,
            .executed = false,
            .reason = "unknown purpose",
            .steps = {},
        };
    }
    return Run(*pack, target, summary);
}

PurposeExecutionResult PurposeRuntime::Run(
    const PurposePack& pack,
    const std::string& target,
    const std::string& summary) const
{
    PurposeExecutionResult result;
    result.purpose_id = pack.id;
    result.allowed = true;
    result.executed = true;

    if (pack.steps.empty())
    {
        result.reason = "purpose has no declared steps";
        return result;
    }

    active_purpose_id_ = pack.id;
    LogAction(pack.id, "purpose_run", target, summary.empty() ? pack.description : summary, "approved");

    for (const auto& step : pack.steps)
    {
        PurposeExecutionStepResult step_result;
        step_result.action = step.action;
        step_result.target = step.target.empty() ? target : step.target;

        const auto eval = CanRun(pack, step.action, step_result.target, summary);
        step_result.allowed = eval.allowed;
        step_result.reason = eval.reason;
        result.steps.push_back(step_result);

        if (!eval.allowed)
        {
            result.allowed = false;
            result.executed = false;
            result.reason = eval.reason;
            LogAction(pack.id, step.action, step_result.target, eval.reason, "blocked");
            return result;
        }

        LogAction(pack.id, step.action, step_result.target, step.detail.empty() ? "step_allowed" : step.detail, "allowed");
    }

    result.reason = "purpose_plan_approved";
    return result;
}

PurposeEvaluation PurposeRuntime::CanRun(
    const PurposePack& pack,
    const std::string& action,
    const std::string& target,
    const std::string& summary) const
{
    (void)summary;
    PurposeEvaluation eval;

    const auto action_name = CanonicalizeActionName(action);
    if (action_name.empty())
    {
        eval.reason = "empty action";
        return eval;
    }

    if (!ListContains(pack.allowed_tools, action_name))
    {
        eval.reason = "tool/action not allowed for purpose " + pack.id;
        return eval;
    }

    if (!target.empty())
    {
        if (!HostAllowed(pack, target))
        {
            eval.reason = "target host not permitted for purpose " + pack.id;
            return eval;
        }
        if (!AppAllowed(pack, target))
        {
            eval.reason = "target app or URL is not permitted for purpose " + pack.id;
            return eval;
        }
    }

    eval.allowed = true;
    return eval;
}

std::string PurposeRuntime::Trim(std::string value)
{
    return TrimView(std::move(value));
}

std::string PurposeRuntime::ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string PurposeRuntime::NormalizeScalar(const std::string& value)
{
    auto trimmed = StripQuotes(TrimView(value));
    return trimmed;
}

std::string PurposeRuntime::CanonicalizeActionName(const std::string& value)
{
    auto normalized = ToLower(Trim(value));
    std::string out;
    out.reserve(normalized.size());
    for (char ch : normalized)
    {
        if (ch == '.' || ch == '-' || ch == ' ' || ch == '/' || ch == ':')
        {
            out += '_';
        }
        else
        {
            out += ch;
        }
    }
    while (!out.empty() && out.front() == '_')
    {
        out.erase(out.begin());
    }
    while (!out.empty() && out.back() == '_')
    {
        out.pop_back();
    }
    return out;
}

std::string PurposeRuntime::ExtractHost(const std::string& target)
{
    const auto lowered = ToLower(Trim(target));
    if (lowered.empty())
    {
        return {};
    }

    const auto scheme_pos = lowered.find("://");
    const std::string effective = scheme_pos == std::string::npos ? lowered : lowered.substr(scheme_pos + 3);
    const auto slash_pos = effective.find('/');
    const auto host = slash_pos == std::string::npos ? effective : effective.substr(0, slash_pos);
    if (host.empty())
    {
        return {};
    }
    return host;
}

bool PurposeRuntime::ListContains(const std::vector<std::string>& values, const std::string& value)
{
    const auto needle = CanonicalizeActionName(value);
    for (const auto& entry : values)
    {
        if (CanonicalizeActionName(entry) == needle)
        {
            return true;
        }
    }
    return false;
}

bool PurposeRuntime::HostAllowed(const PurposePack& pack, const std::string& target)
{
    if (pack.allowed_hosts.empty())
    {
        return true;
    }

    const auto host = ExtractHost(target);
    if (host.empty())
    {
        return true;
    }

    const auto lowered_host = ToLower(host);
    for (const auto& allowed : pack.allowed_hosts)
    {
        if (ToLower(allowed) == lowered_host)
        {
            return true;
        }
    }
    return false;
}

bool PurposeRuntime::AppAllowed(const PurposePack& pack, const std::string& target)
{
    if (pack.allowed_apps.empty())
    {
        return true;
    }

    const auto lowered = ToLower(Trim(target));
    const auto host = ExtractHost(target);
    if (!host.empty())
    {
        const auto allowed_by_host = HostAllowed(pack, host);
        if (allowed_by_host)
        {
            return true;
        }
    }

    for (const auto& allowed : pack.allowed_apps)
    {
        if (ToLower(allowed) == lowered)
        {
            return true;
        }
    }
    return false;
}

bool PurposeRuntime::ParseBool(const std::string& value)
{
    const auto normalized = ToLower(Trim(value));
    return normalized == "1" || normalized == "true" || normalized == "yes" || normalized == "on";
}

PurposeRisk PurposeRuntime::ParseRisk(const std::string& value)
{
    const auto normalized = ToLower(Trim(value));
    if (normalized == "critical") return PurposeRisk::Critical;
    if (normalized == "high") return PurposeRisk::High;
    if (normalized == "medium") return PurposeRisk::Medium;
    return PurposeRisk::Low;
}

} // namespace sovereign::product
