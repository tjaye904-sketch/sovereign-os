#include "desktop_operator.hpp"

#include "sovereign_host.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

bool HasHeadedDesktop()
{
    const char* display = std::getenv("DISPLAY");
    const char* wayland = std::getenv("WAYLAND_DISPLAY");
    return (display != nullptr && display[0] != '\0') ||
           (wayland != nullptr && wayland[0] != '\0');
}

bool IsHeadlessExecutionMode()
{
    const char* mode = std::getenv("SOVEREIGN_SYS_HEADLESS");
    return mode != nullptr && std::string(mode) == "1";
}

std::string CanonicalizeActionName(const std::string& value)
{
    std::string out;
    out.reserve(value.size());
    for (char ch : value)
    {
        if (ch == '.' || ch == '-' || ch == ' ' || ch == '/' || ch == ':')
        {
            out.push_back('_');
        }
        else
        {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
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

std::string EscapeForShell(const std::string& input)
{
    std::string escaped;
    escaped.reserve(input.size() + 2);
    escaped.push_back('\'');
    for (const char ch : input)
    {
        if (ch == '\'')
        {
            escaped += "'\"'\"'";
        }
        else
        {
            escaped.push_back(ch);
        }
    }
    escaped.push_back('\'');
    return escaped;
}

std::string BuildActionStamp()
{
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    const auto secs = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    std::ostringstream oss;
    oss << secs;
    return oss.str();
}

bool WriteDesktopEvidence(
    const sovereign::product::SovereignHost& host,
    const std::string& action,
    const std::string& summary,
    const std::string& mode,
    std::string& report)
{
    const auto evidence_dir = host.DataRoot() / "desktop";
    std::filesystem::create_directories(evidence_dir);
    const auto evidence_path = evidence_dir / ("desktop_action_" + action + "_" + BuildActionStamp() + ".log");

    std::ofstream log(evidence_path, std::ios::out | std::ios::trunc);
    if (!log.is_open())
    {
        report = "desktop action approved but evidence logging failed for action: " + action;
        return false;
    }

    log << "mode=" << mode << '\n';
    log << "action=" << action << '\n';
    log << "summary=" << summary << '\n';
    log << "status=approved" << '\n';
    log.flush();

    report = "desktop action approved; executed in " + mode + " mode; evidence recorded at " + evidence_path.string();
    return true;
}

std::string ExtractExecutionTarget(const std::string& summary)
{
    std::string candidate = summary;
    if (candidate.empty())
    {
        return {};
    }

    const std::array<std::string, 4> delimiters = {" :: ", " | ", " - ", ": "};
    for (const auto& delimiter : delimiters)
    {
        const auto pos = candidate.find(delimiter);
        if (pos == std::string::npos)
        {
            continue;
        }
        const auto first = candidate.substr(0, pos);
        if (!first.empty() && (first.find("://") != std::string::npos || first.find('/') != std::string::npos || first.find('.') != std::string::npos || first.find(' ') == std::string::npos))
        {
            return first;
        }
    }

    if (candidate.find("://") != std::string::npos ||
        candidate.find('/') != std::string::npos ||
        candidate.find('.') != std::string::npos ||
        candidate.find(' ') == std::string::npos)
    {
        return candidate;
    }
    return {};
}

void AppendDesktopAudit(
    sovereign::product::SovereignHost& host,
    const sovereign::PrincipalId& actor,
    const std::string& action,
    const std::string& operation,
    const std::string& detail)
{
    sovereign::audit::AuditEntry entry;
    entry.category = "desktop";
    entry.action = operation;
    entry.actor = actor;
    entry.subject = action;
    entry.detail = detail;
    static_cast<void>(host.Audit().Append(entry));
}

} // namespace

namespace sovereign::product
{

DesktopOperatorGateway::DesktopOperatorGateway(SovereignHost& host)
    : host_(&host)
{
}

std::string DesktopOperatorGateway::ToHex(const std::array<std::uint8_t, 16>& bytes)
{
    static constexpr char kHex[] = "0123456789abcdef";
    std::string out(32, '0');
    for (std::size_t i = 0; i < 16; ++i)
    {
        out[i * 2] = kHex[(bytes[i] >> 4) & 0xF];
        out[i * 2 + 1] = kHex[bytes[i] & 0xF];
    }
    return out;
}

bool DesktopOperatorGateway::FromHex(const std::string& hex, std::array<std::uint8_t, 16>& bytes)
{
    if (hex.size() != 32)
    {
        return false;
    }

    auto to_nibble = [](char ch) -> int {
        if (ch >= '0' && ch <= '9')
            return ch - '0';
        if (ch >= 'a' && ch <= 'f')
            return ch - 'a' + 10;
        if (ch >= 'A' && ch <= 'F')
            return ch - 'A' + 10;
        return -1;
    };

    for (std::size_t i = 0; i < 16; ++i)
    {
        const int hi = to_nibble(hex[i * 2]);
        const int lo = to_nibble(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0)
        {
            return false;
        }
        bytes[i] = static_cast<std::uint8_t>((hi << 4) | lo);
    }
    return true;
}

bool DesktopOperatorGateway::IsAllowedAction(const std::string& action)
{
    const auto normalized = CanonicalizeActionName(action);
    return normalized == "desktop_snapshot" ||
           normalized == "desktop_list_windows" ||
           normalized == "desktop_focus" ||
           normalized == "desktop_input" ||
           normalized == "browser_open" ||
           normalized == "browser_snapshot" ||
           normalized == "browser_act" ||
           normalized == "os_notify" ||
           normalized == "os_files_open" ||
           normalized == "app_open" ||
           normalized == "app_launch";
}

bool DesktopOperatorGateway::RequestAction(
    const PrincipalId& actor,
    const std::string& action,
    const std::string& target,
    const std::string& summary,
    std::string& decision_hex)
{
    if (host_ == nullptr || action.empty())
    {
        return false;
    }

    const auto execution_target = target.empty() ? summary : target;
    const std::string evidence = execution_target.empty() ? summary : execution_target + " :: " + summary;
    const auto request = host_->Governance().RequestDecision(actor, action, evidence, "desktop-operator");
    if (!request.Ok())
    {
        return false;
    }

    AppendDesktopAudit(*host_, actor, action, "request", evidence);
    decision_hex = ToHex(request.value.bytes);
    return true;
}

bool DesktopOperatorGateway::Approve(
    const PrincipalId& actor,
    const std::string& decision_hex,
    const std::string& reason)
{
    if (host_ == nullptr)
    {
        return false;
    }

    governance::DecisionId id{};
    if (!FromHex(decision_hex, id.bytes))
    {
        return false;
    }

    const auto result = host_->Governance().Approve(actor, true, id, reason);
    if (result.Ok())
    {
        AppendDesktopAudit(*host_, actor, host_->Governance().Get(id).value.action, "approve", reason);
    }
    return result.Ok();
}

bool DesktopOperatorGateway::Reject(
    const PrincipalId& actor,
    const std::string& decision_hex,
    const std::string& reason)
{
    if (host_ == nullptr)
    {
        return false;
    }

    governance::DecisionId id{};
    if (!FromHex(decision_hex, id.bytes))
    {
        return false;
    }

    const auto result = host_->Governance().Reject(actor, true, id, reason);
    if (result.Ok())
    {
        AppendDesktopAudit(*host_, actor, host_->Governance().Get(id).value.action, "reject", reason);
    }
    return result.Ok();
}

bool DesktopOperatorGateway::ExecuteApprovedAction(
    const std::string& decision_hex,
    std::string& report)
{
    if (host_ == nullptr)
    {
        return false;
    }

    governance::DecisionId id{};
    if (!FromHex(decision_hex, id.bytes))
    {
        return false;
    }

    const auto decision = host_->Governance().Get(id);
    if (!decision.Ok())
    {
        return false;
    }

    if (decision.value.status != governance::DecisionStatus::Approved)
    {
        AppendDesktopAudit(*host_, decision.value.requester, decision.value.action, "execute_blocked", "decision not approved");
        return false;
    }

    if (!IsAllowedAction(decision.value.action))
    {
        report = "desktop action not allowed by safe operator policy: " + decision.value.action;
        AppendDesktopAudit(*host_, decision.value.requester, decision.value.action, "execute_blocked", report);
        return false;
    }

    const auto normalized_action = CanonicalizeActionName(decision.value.action);
    const bool be_headless = !HasHeadedDesktop() || IsHeadlessExecutionMode();
    if (be_headless)
    {
        const bool ok = WriteDesktopEvidence(
            *host_,
            normalized_action,
            decision.value.summary,
            IsHeadlessExecutionMode() ? "headless-safe" : "safe-headless",
            report);
        AppendDesktopAudit(*host_, decision.value.requester, normalized_action, ok ? "execute" : "execute_failed", report);
        return ok;
    }

    std::string command;
    const std::string target = EscapeForShell(ExtractExecutionTarget(decision.value.summary));
    const auto safe_target = ExtractExecutionTarget(decision.value.summary);
    const auto output_path = (host_->DataRoot() / "desktop" / ("capture_" + BuildActionStamp() + ".png")).string();
    if (normalized_action == "desktop_snapshot" || normalized_action == "browser_snapshot")
    {
        const auto output = EscapeForShell(output_path);
        command = "mkdir -p " + EscapeForShell((host_->DataRoot() / "desktop").string()) + " && (command -v gnome-screenshot >/dev/null 2>&1 && gnome-screenshot -f " + output + " >/dev/null 2>&1 || command -v scrot >/dev/null 2>&1 && scrot -q 100 " + output + " >/dev/null 2>&1 || command -v import >/dev/null 2>&1 && import -window root " + output + " >/dev/null 2>&1)";
    }
    else if (normalized_action == "desktop_list_windows")
    {
        command = "command -v wmctrl >/dev/null 2>&1 && wmctrl -l";
    }
    else if (normalized_action == "desktop_focus")
    {
        command = "command -v wmctrl >/dev/null 2>&1 && wmctrl -a " + (safe_target.empty() ? "" : EscapeForShell(safe_target));
    }
    else if (normalized_action == "browser_open" || normalized_action == "app_open" || normalized_action == "app_launch")
    {
        if (safe_target.empty())
        {
            report = "desktop action missing a valid target for execution: " + decision.value.action;
            return false;
        }
        command = "command -v xdg-open >/dev/null 2>&1 && xdg-open " + EscapeForShell(safe_target);
    }
    else if (normalized_action == "desktop_input")
    {
        if (safe_target.empty())
        {
            report = "desktop input action missing a valid target payload: " + decision.value.action;
            return false;
        }
        command = "command -v xdotool >/dev/null 2>&1 && xdotool type --clearmodifiers " + EscapeForShell(safe_target);
    }
    else
    {
        report = "desktop action not supported for execution: " + decision.value.action;
        return false;
    }

    const int rc = std::system(command.c_str());
    if (rc != 0)
    {
        report = "desktop action execution failed for action: " + decision.value.action + " (shell exit=" + std::to_string(rc) + ")";
        AppendDesktopAudit(*host_, decision.value.requester, decision.value.action, "execute_failed", report);
        return false;
    }

    const bool ok = WriteDesktopEvidence(
        *host_,
        decision.value.action,
        decision.value.summary,
        "headed-execution",
        report);
    AppendDesktopAudit(*host_, decision.value.requester, decision.value.action, ok ? "execute" : "execute_failed", report);
    return ok;
}

DesktopOperatorStatus DesktopOperatorGateway::Status(std::size_t audit_limit) const
{
    DesktopOperatorStatus out;
    out.has_display = HasHeadedDesktop();
    if (IsHeadlessExecutionMode() || !out.has_display)
    {
        out.mode = "headless";
    }
    else
    {
        out.mode = "headed";
    }

    if (host_ == nullptr)
    {
        return out;
    }

    const auto cu = host_->Capabilities().IsEnabled("host.computer_use");
    out.computer_use_enabled = cu.Ok() && cu.value;
    out.active_purpose_id = host_->Purpose().CurrentPurposeId();

    const auto pending = host_->Governance().ListPending();
    if (pending.Ok())
    {
        out.pending_decisions = pending.value.size();
    }

    std::error_code ec;
    const auto evidence_dir = host_->DataRoot() / "desktop";
    if (std::filesystem::exists(evidence_dir, ec) && std::filesystem::is_directory(evidence_dir, ec))
    {
        for (const auto& entry : std::filesystem::directory_iterator(evidence_dir, ec))
        {
            if (entry.is_regular_file(ec))
            {
                ++out.evidence_files;
            }
        }
    }

    const auto desktop_audits = host_->Audit().ListByCategory("desktop", audit_limit);
    if (desktop_audits.Ok())
    {
        for (const auto& item : desktop_audits.value)
        {
            out.recent_audit_events.push_back(
                "desktop:" + item.action + " subject=" + item.subject + " detail=" + item.detail);
        }
    }

    const auto gov_audits = host_->Audit().ListByCategory("governance", audit_limit);
    if (gov_audits.Ok())
    {
        for (const auto& item : gov_audits.value)
        {
            if (out.recent_audit_events.size() >= audit_limit)
            {
                break;
            }
            if (item.action.rfind("purpose.", 0) == 0)
            {
                out.recent_audit_events.push_back(
                    "governance:" + item.action + " subject=" + item.subject + " detail=" + item.detail);
            }
        }
    }

    return out;
}
} // namespace sovereign::product
