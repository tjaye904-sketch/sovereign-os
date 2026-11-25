#include "capability_gate.hpp"
#include "node_registry.hpp"
#include "capability_registry.hpp"
#include "memory_facets.hpp"
#include "secret_manager.hpp"
// Sovereign local CLI — in-process SovereignHost surface (not a network gateway).

#include "sovereign_host.hpp"
#include "service_orchestrator.hpp"
#include "transparency_report.hpp"
#include "domain_tool_runner.hpp"
#include "identity_verifier.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{

std::filesystem::path DefaultDataRoot()
{
    if (const char* root = std::getenv("SOVEREIGN_DATA_ROOT");
        root != nullptr && root[0] != '\0')
    {
        return std::filesystem::path(root);
    }
    const char* home = std::getenv("HOME");
    if (home != nullptr && home[0] != '\0')
    {
        return std::filesystem::path(home) / ".local" / "share" / "sovereign";
    }
    return std::filesystem::temp_directory_path() / "sovereign";
}


namespace {
// Returns optional exit code if handled; -1 if not this family of commands.
int TryCapabilityAndNodeCommands(sovereign::product::SovereignHost& host, int argc, char** argv, const std::string& cmd)
{

    if (cmd == "capability-gate-check") {
        if (argc < 3) { std::cerr << "usage: capability-gate-check <action-or-id>\n"; return 2; }
        sovereign::capability::CapabilityRegistry caps;
        if (caps.Open(host.DataRoot()) != sovereign::capability::CapabilityError::Ok) {
            std::cerr << "capability open failed\n"; return 1;
        }
        (void)caps.SeedDefaults();
        sovereign::capability::CapabilityGate gate(caps);
        std::string key = argv[2];
        auto d = (key.find('.') != std::string::npos && key.rfind("host.", 0) == 0)
            ? gate.Require(key)
            : gate.RequireForAction(key);
        // also allow raw id without host. prefix
        if (!d.allowed && key.find('.') != std::string::npos)
            d = gate.Require(key);
        std::cout << "capability_id=" << d.capability_id
                  << " allowed=" << (d.allowed ? 1 : 0)
                  << " reason=" << d.reason << "\n";
        return d.allowed ? 0 : 3;
    }

    if (cmd == "capability-seed" || cmd == "capability-list" || cmd == "capability-enable"
        || cmd == "capability-disable" || cmd == "capability-check")
    {
        sovereign::capability::CapabilityRegistry caps;
        if (caps.Open(host.DataRoot()) != sovereign::capability::CapabilityError::Ok)
        { std::cerr << "capability open failed\n"; return 1; }
        if (cmd == "capability-seed") {
            auto r = caps.SeedDefaults();
            if (!r.Ok()) { std::cerr << "seed failed\n"; return 1; }
            std::cout << "seeded_added=" << r.value << "\n"; return 0;
        }
        if (cmd == "capability-list") {
            auto r = caps.List();
            if (!r.Ok()) { std::cerr << "list failed\n"; return 1; }
            if (r.value.empty()) { (void)caps.SeedDefaults(); r = caps.List(); }
            for (const auto& c : r.value)
                std::cout << "id=" << c.id << " enabled=" << (c.enabled ? 1 : 0)
                          << " source=" << c.source << " desc=" << c.description << "\n";
            return 0;
        }
        if (argc < 3) { std::cerr << "usage: " << cmd << " <id>\n"; return 2; }
        std::string id = argv[2];
        if (cmd == "capability-enable" || cmd == "capability-disable") {
            auto r = caps.SetEnabled(id, cmd == "capability-enable");
            if (!r.Ok()) { std::cerr << "set failed\n"; return 1; }
            std::cout << "id=" << id << " enabled=" << (cmd == "capability-enable" ? 1 : 0) << "\n";
            return 0;
        }
        auto r = caps.IsEnabled(id);
        if (!r.Ok()) { std::cerr << "check failed\n"; return 1; }
        std::cout << "id=" << id << " enabled=" << (r.value ? 1 : 0) << "\n";
        return 0;
    }
    if (cmd.rfind("node-", 0) != 0 || cmd == "node-execute" || cmd == "node-replace") return -1;
    sovereign::node::NodeRegistry nodes;
    if (nodes.Open(host.DataRoot()) != sovereign::node::NodeError::Ok)
    { std::cerr << "node open failed\n"; return 1; }
    if (cmd == "node-register-local") {
        if (argc < 3) { std::cerr << "usage: node-register-local <name>\n"; return 2; }
        auto r = nodes.RegisterLocal(argv[2]);
        if (!r.Ok()) { std::cerr << "register failed\n"; return 1; }
        std::cout << "node_id=" << sovereign::node::NodeRegistry::IdHex(r.value) << "\n"; return 0;
    }
    if (cmd == "node-register-peer") {
        if (argc < 4) { std::cerr << "usage: node-register-peer <name> <endpoint>\n"; return 2; }
        auto r = nodes.RegisterPeer(argv[2], argv[3]);
        if (!r.Ok()) { std::cerr << "register failed\n"; return 1; }
        std::cout << "node_id=" << sovereign::node::NodeRegistry::IdHex(r.value) << "\n"; return 0;
    }
    if (cmd == "node-list") {
        auto r = nodes.List();
        if (!r.Ok()) { std::cerr << "list failed\n"; return 1; }
        for (const auto& n : r.value)
            std::cout << "name=" << n.name << " kind=" << sovereign::node::KindName(n.kind)
                      << " health=" << sovereign::node::HealthName(n.health)
                      << " ws=" << n.workspace_name << " caps=" << n.capabilities.size() << "\n";
        return 0;
    }
    if (cmd == "node-heartbeat") {
        if (argc < 3) { std::cerr << "usage: node-heartbeat <name>\n"; return 2; }
        auto f = nodes.FindByName(argv[2]);
        if (!f.Ok()) { std::cerr << "not found\n"; return 1; }
        if (!nodes.Heartbeat(f.value.id).Ok()) { std::cerr << "heartbeat failed\n"; return 1; }
        std::cout << "name=" << argv[2] << " health=healthy\n"; return 0;
    }
    if (cmd == "node-bind") {
        if (argc < 4) { std::cerr << "usage: node-bind <name> <workspace>\n"; return 2; }
        auto f = nodes.FindByName(argv[2]);
        if (!f.Ok()) { std::cerr << "not found\n"; return 1; }
        if (!nodes.BindWorkspace(f.value.id, argv[3]).Ok()) { std::cerr << "bind failed\n"; return 1; }
        std::cout << "name=" << argv[2] << " workspace=" << argv[3] << "\n"; return 0;
    }
    if (cmd == "node-caps") {
        if (argc < 4) { std::cerr << "usage: node-caps <name> <cap,cap,...>\n"; return 2; }
        auto f = nodes.FindByName(argv[2]);
        if (!f.Ok()) { std::cerr << "not found\n"; return 1; }
        std::vector<std::string> caps; std::string s = argv[3], cur;
        for (char c : s) {
            if (c == ',') { if (!cur.empty()) caps.push_back(cur); cur.clear(); }
            else cur.push_back(c);
        }
        if (!cur.empty()) caps.push_back(cur);
        if (!nodes.SetCapabilities(f.value.id, caps).Ok()) { std::cerr << "caps failed\n"; return 1; }
        std::cout << "name=" << argv[2] << " caps=" << caps.size() << "\n"; return 0;
    }
    if (cmd == "node-events") {
        auto r = nodes.RecentEvents(30);
        if (!r.Ok()) { std::cerr << "events failed\n"; return 1; }
        for (const auto& e : r.value) std::cout << e << "\n";
        return 0;
    }
    std::cerr << "unknown node command\n";
    return 2;
}
} // namespace

void PrintUsage()
{
    std::cerr
        << "sovereign-cli — local Sovereign host commands\n"
        << "Usage:\n"
        << "  sovereign-cli status\n"
        << "  sovereign-cli bootstrap <name> <secret>\n"
        << "  sovereign-cli service-create <name>\n"
        << "  sovereign-cli machine-create <name>\n"
        << "  sovereign-cli login <name> <secret> [totp]\n"
        << "  sovereign-cli workspace-create <name> <kind>\n"
        << "  sovereign-cli workspace-list\n"
        << "  sovereign-cli workspace-activate <name-or-hex>\n"
        << "  sovereign-cli workspace-suspend <name-or-hex>\n"
        << "  sovereign-cli workspace-status [name]\n"

        << "  sovereign-cli memory-add <tag> <body...>\n"
        << "  sovereign-cli memory-list [tag]\n"
        << "  sovereign-cli recommend <action> <summary>\n"
        << "  sovereign-cli loop [--auto-approve] <goal...>\n"
        << "  sovereign-cli approve <decision-hex> <reason>\n"
        << "  sovereign-cli reject <decision-hex> <reason>\n"
        << "  sovereign-cli override <decision-hex> <reason>\n"
        << "  sovereign-cli rule-set <action> <auto:0|1> <min-approve-role>\n"
        << "  sovereign-cli logout\n"
        << "  sovereign-cli session-revoke\n"
        << "  sovereign-cli mfa-status\n"
        << "  sovereign-cli mfa-enroll-begin\n"
        << "  sovereign-cli mfa-enroll-confirm <code>\n"
        << "  sovereign-cli mfa-recovery-generate\n"
        << "  sovereign-cli execute <action> <decision-hex> [--mfa <code>]\n"
        << "  sovereign-cli principal-suspend <principal-hex>\n"
        << "  sovereign-cli principal-revoke <principal-hex>\n"
        << "  sovereign-cli operator-create <name> <secret>\n"
        << "  sovereign-cli role-add <principal-hex> <role>\n"
        << "  sovereign-cli role-remove <principal-hex> <role>\n"
        << "  sovereign-cli roles <principal-hex>\n"
        << "  sovereign-cli decision-get <decision-hex>\n"
        << "  sovereign-cli change-credential <old> <new>\n"
        << "  sovereign-cli principals\n"
        << "  sovereign-cli audit-list [category]\n"
        << "  sovereign-cli brief\n"
        << "  sovereign-cli goal-add <priority> <title...>\n"
        << "  sovereign-cli goal-list\n"
        << "  sovereign-cli goal-complete <goal-hex>\n"
        << "  sovereign-cli memory-search <query>\n"
        << "  sovereign-cli workspace-init-domains\n"
        << "  sovereign-cli agent-add-brief [interval-seconds]\n"
        << "  sovereign-cli agent-list\n"
        << "  sovereign-cli agent-tick\n"
        << "  sovereign-cli semantic-add <text...>\n"
        << "  sovereign-cli semantic-query <text...>\n"
        << "  sovereign-cli domain-show [name]\n"
        << "  sovereign-cli domain-chat <domain> <prompt...>\n"
        << "  sovereign-cli peer-export <dest-dir>\n"
        << "  sovereign-cli peer-import <bundle-dir>\n"
        << "  sovereign-cli inspect [section]\n"
        << "  sovereign-cli ask <question...>\n"
        << "  sovereign-cli pending-decisions\n"
        << "  sovereign-cli archive-store <label> <hash>\n"
        << "  sovereign-cli sync-export <channel> <ref>\n"
        << "  sovereign-cli models\n"
        << "  sovereign-cli chat <prompt...>\n"
        << "  sovereign-cli code-assist [--save] [--json] [--file path]... -- <prompt...>\n"
        << "  sovereign-cli code-assist <prompt...>\n"
        << "  sovereign-cli project-summary [--save] [dir]\n"
        << "  sovereign-cli review-diff [--save] <diff-file>\n"
        << "  sovereign-cli policy-check <role> <action>\n"
        << "  sovereign-cli purpose-list\n"
        << "  sovereign-cli purpose-run <purpose-id> [target] [summary...]\n"
        << "  sovereign-cli memory-rank\n"
        << "  sovereign-cli memory-facet-add <facet> <tag> <body...>\n"
        << "  sovereign-cli memory-facet-list [facet]\n"
        << "  sovereign-cli memory-facet-counts\n"

        << "  sovereign-cli memory-prune <min-score>\n"
        << "  sovereign-cli curious <topic...>\n"
        << "  sovereign-cli evolve [--limit N] [path]\n"
        << "  sovereign-cli sandbox-propose [--limit N] [path]\n"
        << "  sovereign-cli sandbox-approve <proposal-id> <approver>\n"
        << "  sovereign-cli plan <goal...>\n"
        << "  sovereign-cli policy-dry-run <role> <action>\n"
        << "  sovereign-cli policy-version\n"
        << "  sovereign-cli policy-rule-set <action> <min-role> <allow|deny>\n"
        << "  sovereign-cli external-request <action> <summary...>\n"
        << "  sovereign-cli external-approve <decision-hex> <reason...>\n"
        << "  sovereign-cli audit-verify\n"
        << "  sovereign-cli secret-put <name> <value>\n"
        << "  sovereign-cli secret-get <name>\n"
        << "  sovereign-cli secret-list\n"
        << "  sovereign-cli secret-exists <name>\n"
        << "  sovereign-cli secret-delete <name>\n"

        << "  sovereign-cli chat-tools <prompt...>\n"
        << "  sovereign-cli execute <action> <decision-hex>\n"
        << "  sovereign-cli capability-seed\n"
        << "  sovereign-cli capability-list\n"
        << "  sovereign-cli capability-enable <id>\n"
        << "  sovereign-cli capability-disable <id>\n"
        << "  sovereign-cli capability-check <id>\n"
        << "  sovereign-cli node-register-local <name>\n"
        << "  sovereign-cli node-register-peer <name> <endpoint>\n"
        << "  sovereign-cli node-list\n"
        << "  sovereign-cli node-heartbeat <name>\n"
        << "  sovereign-cli node-bind <name> <workspace>\n"
        << "  sovereign-cli node-caps <name> <cap,cap,...>\n"
        << "  sovereign-cli node-events\n"
                << "  sovereign-cli capability-gate-check <action-or-id>\n"
                << "  sovereign-cli desktop-status [--json]\n"
                << "  sovereign-cli session-ensure [--json]\n"
                << "  sovereign-cli plan-route [--json] <workspace> <action>\n"
                << "  sovereign-cli node-execute [--json] <workspace> <action> [payload...]\n"
                << "  sovereign-cli core-backup [--json] <snapshot-dir>\n"
                << "  sovereign-cli core-restore [--json] <snapshot-dir>\n"
                << "  sovereign-cli profile-check [--json]\n"
                << "  sovereign-cli fleet-status [--json]\n"
                << "  sovereign-cli node-replace [--json] <workspace> <new-node-name> [endpoint]\n"
                << "  sovereign-cli verify-outcome [--json] <action> <expected> <observed...>\n"
                << "  sovereign-cli goal-execute [--json] <workspace> <goal...>\n"
                << "  sovereign-cli curiosity-probe [--json] <probe-action> <topic...>\n"
                << "  sovereign-cli inventory-sync [--apps a,b,c]\n"
                << "  sovereign-cli app-check [--json] <app-id>\n"
                << "  sovereign-cli purpose-seed\n"
                << "  sovereign-cli purpose-inspect [--json] <purpose-id>\n"
                << "  sovereign-cli desktop-drive [--json] [--allow-headless] <op> <app> [payload...]\n"
        << "Env: SOVEREIGN_DATA_ROOT, OLLAMA_HOST, SOVEREIGN_OLLAMA_MODEL\n";
}

std::string ToHex(const std::array<std::uint8_t, 16>& b)
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

std::string JsonEscape(const std::string& input)
{
    std::string out;
    out.reserve(input.size());
    for (char ch : input)
    {
        switch (ch)
        {
            case '\\':
                out += "\\\\";
                break;
            case '"':
                out += "\\\"";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(ch) < 0x20)
                {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x",
                                  static_cast<unsigned int>(static_cast<unsigned char>(ch)));
                    out += buf;
                }
                else
                {
                    out += ch;
                }
                break;
        }
    }
    return out;
}

template <typename T>
void PrintIdentityFailure(const std::string& action, const sovereign::identity::Result<T>& result)
{
    std::cerr << action << " failed: "
              << sovereign::identity::IdentityErrorToString(result.error) << '\n';
}

bool FromHex(const std::string& h, std::array<std::uint8_t, 16>& b)
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

std::filesystem::path SessionFilePath(const std::filesystem::path& data_root)
{
    return data_root / "cli_session.hex";
}

void SaveSession(
    const std::filesystem::path& data_root,
    const std::array<std::uint8_t, 16>& session_id)
{
    std::ofstream out(SessionFilePath(data_root), std::ios::trunc);
    out << ToHex(session_id) << '\n';
}

bool LoadSession(
    const std::filesystem::path& data_root,
    std::array<std::uint8_t, 16>& session_id)
{
    std::ifstream in(SessionFilePath(data_root));
    std::string hex;
    if (!(in >> hex))
        return false;
    return FromHex(hex, session_id);
}


struct CmdTimer
{
    std::filesystem::path data_root;
    std::string cmd;
    std::chrono::steady_clock::time_point t0{
        std::chrono::steady_clock::now()};
    bool active{true};

    ~CmdTimer()
    {
        if (!active || cmd.empty() || cmd == "inspect")
            return;
        using namespace std::chrono;
        const auto ms =
            duration_cast<milliseconds>(steady_clock::now() - t0).count();
        std::error_code ec;
        const auto dir = data_root / "metrics";
        std::filesystem::create_directories(dir, ec);
        std::ofstream out(dir / "cmd_times.log", std::ios::app);
        if (!out)
            return;
        const auto wall = duration_cast<seconds>(
                              system_clock::now().time_since_epoch())
                              .count();
        out << wall << ' ' << cmd << ' ' << ms << '\n';
    }
};

} // namespace

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        PrintUsage();
        return 2;
    }

    sovereign::product::HostConfig cfg;
    cfg.data_root = DefaultDataRoot();
    if (const char* ollama = std::getenv("OLLAMA_HOST");
        ollama != nullptr && ollama[0] != '\0')
    {
        cfg.ollama_base = ollama;
    }

    sovereign::product::SovereignHost host(cfg);
    if (argc >= 3 && std::string(argv[1]) == "core-restore")
    {
        bool as_json = false;
        int idx = 2;
        if (std::string(argv[idx]) == "--json" && argc >= 4) { as_json = true; ++idx; }
        const std::filesystem::path snap_dir = argv[idx];
        const auto rep = sovereign::archive::ArchiveService::RestoreCoreSnapshot(snap_dir, cfg.data_root);
        if (rep.ok && host.Open())
        {
            sovereign::audit::AuditEntry ae;
            ae.category = "ha";
            ae.action = "core-restore";
            ae.subject = snap_dir.string();
            ae.detail = "ok=1 checksum=" + rep.checksum_hex;
            (void)host.Audit().Append(ae);
            host.Close();
        }
        if (as_json)
            std::cout << "{\"ok\":" << (rep.ok ? "true" : "false") << ",\"op\":\"core-restore\",\"files\":" << rep.file_count
                      << ",\"bytes\":" << rep.total_bytes << ",\"checksum\":\"" << rep.checksum_hex << "\",\"detail\":\"" << rep.detail << "\"}\n";
        else
            std::cout << "ok=" << (rep.ok ? "1" : "0") << " op=core-restore files=" << rep.file_count << " checksum=" << rep.checksum_hex << "\n";
        return rep.ok ? 0 : 1;
    }

    if (!host.Open())
    {
        std::cerr << "failed to open host at " << cfg.data_root << '\n';
        return 1;
    }

    const std::string cmd = argv[1];
    {
        int _sov_l2 = TryCapabilityAndNodeCommands(host, argc, argv, cmd);
        if (_sov_l2 >= 0) return _sov_l2;
    }

    CmdTimer cmd_timer{cfg.data_root, cmd};

    if (cmd == "status")
    {
        const bool json = (argc >= 3 && std::string(argv[2]) == "--json");
        const auto models = host.Models().Health();
        const auto ws = host.Workspace().List();
        const auto ollama = (models.Ok() && models.value) ? "up" : "down";
        const auto workspaces = (ws.Ok() ? ws.value.size() : 0);
        const auto argon2 = sovereign::identity::DescribeArgon2Params();
        const auto embed_model = std::string(
            std::getenv("SOVEREIGN_EMBED_MODEL") && std::getenv("SOVEREIGN_EMBED_MODEL")[0]
                ? std::getenv("SOVEREIGN_EMBED_MODEL")
                : "nomic-embed-text (fallback hash-v1)");

        if (json)
        {
            std::cout << "{\n"
                      << "  \"ok\": true,\n"
                      << "  \"data_root\": \"" << JsonEscape(host.DataRoot().string()) << "\",\n"
                      << "  \"ollama\": \"" << JsonEscape(ollama) << "\",\n"
                      << "  \"workspaces\": " << workspaces << ",\n"
                      << "  \"argon2\": \"" << JsonEscape(argon2) << "\",\n"
                      << "  \"surface\": \"cli-only\",\n"
                      << "  \"sync\": \"peer-bundle\",\n"
                      << "  \"hermes\": \"separate\",\n"
                      << "  \"model\": \"" << JsonEscape(host.DefaultModel()) << "\",\n"
                      << "  \"embed_model\": \"" << JsonEscape(embed_model) << "\"\n"
                      << "}\n";
            host.Close();
            return 0;
        }

        std::cout << "data_root=" << host.DataRoot() << '\n';
        std::cout << "ollama=" << ollama << '\n';
        std::cout << "workspaces=" << workspaces << '\n';
        std::cout << "argon2=" << argon2 << '\n';
        std::cout << "surface=cli-only\n";
        std::cout << "sync=peer-bundle\n";
        std::cout << "hermes=separate\n";
        std::cout << "model=" << host.DefaultModel() << '\n';
        std::cout << "embed_model=" << embed_model << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "purpose-list")
    {
        const auto& packs = host.Purpose().BuiltinPacks();
        if (packs.empty())
        {
            std::cout << "no_purposes\n";
            host.Close();
            return 0;
        }
        for (const auto& pack : packs)
        {
            std::cout << pack.id << " | " << pack.description << " | risk="
                      << static_cast<int>(pack.risk) << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "purpose-run" && argc >= 3)
    {
        {
            auto _d = host.Gate().RequireForAction("purpose-run");
            if (!_d.allowed) {
                host.Purpose().LogAction(argv[2], "purpose_run", "", _d.reason, "blocked");
                std::cerr << "capability_denied id=" << _d.capability_id
                          << " reason=" << _d.reason << "\n";
                host.Close();
                return 3;
            }
        }

        std::string target;
        std::string summary;
        if (argc >= 4)
        {
            target = argv[3];
        }
        if (argc >= 5)
        {
            for (int i = 4; i < argc; ++i)
            {
                if (i > 4)
                    summary.push_back(' ');
                summary += argv[i];
            }
        }

        const auto result = host.Purpose().Run(argv[2], target, summary);
        if (!result.allowed)
        {
            std::cerr << "purpose_run_failed purpose=" << argv[2]
                      << " reason=" << result.reason << '\n';
            host.Close();
            return 1;
        }

        std::cout << "purpose_id=" << result.purpose_id << '\n';
        std::cout << "allowed=1\n";
        std::cout << "status=" << result.reason << '\n';
        std::cout << "steps=" << result.steps.size() << '\n';
        for (std::size_t i = 0; i < result.steps.size(); ++i)
        {
            const auto& step = result.steps[i];
            std::cout << "step" << (i + 1) << " action=" << step.action;
            if (!step.target.empty())
                std::cout << " target=" << step.target;
            std::cout << " allowed=" << (step.allowed ? "1" : "0");
            if (!step.reason.empty())
                std::cout << " reason=" << step.reason;
            std::cout << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "bootstrap" && argc >= 4)
    {
        auto r = host.Identity().BootstrapOwner(argv[2], argv[3]);
        if (!r.Ok())
        {
            PrintIdentityFailure("bootstrap", r);
            host.Close();
            return 1;
        }
        std::cout << "owner_id=" << ToHex(r.value.bytes) << '\n';
        {
            sovereign::audit::AuditEntry e;
            e.category = "identity";
            e.action = "bootstrap_owner";
            e.actor = r.value;
            e.subject = argv[2];
            e.detail = "owner created";
            static_cast<void>(host.Audit().Append(e));
        }
        host.Close();
        return 0;
    }

    if (cmd == "login" && argc >= 4)
    {
        sovereign::identity::Result<sovereign::identity::Session> r;
        if (argc >= 5)
            r = host.Identity().AuthenticateWithMfa(argv[2], argv[3], argv[4]);
        else
            r = host.Identity().Authenticate(argv[2], argv[3]);
        if (!r.Ok())
        {
            if (r.error == sovereign::identity::IdentityError::MfaRequired)
                std::cerr << "mfa_required: login <name> <secret> <totp-code>\n";
            else
                PrintIdentityFailure("login", r);
            host.Close();
            return 1;
        }
        std::cout << "session_id=" << ToHex(r.value.id.bytes) << '\n';
        SaveSession(cfg.data_root, r.value.id.bytes);
        {
            sovereign::audit::AuditEntry e;
            e.category = "identity";
            e.action = "login";
            e.actor = r.value.principal_id;
            e.subject = argv[2];
            e.detail = "session issued";
            static_cast<void>(host.Audit().Append(e));
        }
        host.Close();
        return 0;
    }

    if (cmd == "workspace-create" && argc >= 4)
    {
        auto r = host.Workspace().Create(argv[2], argv[3]);
        if (!r.Ok())
        {
            std::cerr << "workspace-create failed\n";
            host.Close();
            return 1;
        }
        std::cout << "workspace_id=" << ToHex(r.value.bytes) << '\n';
        host.Close();
        return 0;
    }

    
    if (cmd == "workspace-activate" || cmd == "workspace-suspend" || cmd == "workspace-status")
    {
        auto resolve = [&](const std::string& key) -> sovereign::workspace::Result<sovereign::workspace::Workspace> {
            sovereign::workspace::Result<sovereign::workspace::Workspace> r;
            if (key.size() == 32)
            {
                sovereign::workspace::WorkspaceId id{};
                // try hex via List scan
                auto all = host.Workspace().List();
                if (!all.Ok()) { r.error = all.error; return r; }
                for (const auto& w : all.value)
                {
                    bool match = true;
                    static const char* hx = "0123456789abcdef";
                    std::string h(32, '0');
                    for (size_t i = 0; i < 16; ++i) {
                        h[i*2] = hx[(w.id.bytes[i] >> 4) & 0xF];
                        h[i*2+1] = hx[w.id.bytes[i] & 0xF];
                    }
                    if (h == key) { r.value = w; return r; }
                }
            }
            return host.Workspace().FindByName(key);
        };
        
    if (cmd == "capability-seed" || cmd == "capability-list" || cmd == "capability-enable"
        || cmd == "capability-disable" || cmd == "capability-check")
    {
        sovereign::capability::CapabilityRegistry caps;
        if (caps.Open(host.DataRoot()) != sovereign::capability::CapabilityError::Ok)
        {
            std::cerr << "capability open failed\n";
            return 1;
        }
        
    if (cmd.rfind("node-", 0) == 0)
    {
        sovereign::node::NodeRegistry nodes;
        if (nodes.Open(host.DataRoot()) != sovereign::node::NodeError::Ok)
        {
            std::cerr << "node open failed\n";
            return 1;
        }
        if (cmd == "node-register-local")
        {
            if (argc < 3) { std::cerr << "usage: node-register-local <name>\n"; return 2; }
            auto r = nodes.RegisterLocal(argv[2]);
            if (!r.Ok()) { std::cerr << "register failed\n"; return 1; }
            std::cout << "node_id=" << sovereign::node::NodeRegistry::IdHex(r.value) << "\n";
            return 0;
        }
        if (cmd == "node-register-peer")
        {
            if (argc < 4) { std::cerr << "usage: node-register-peer <name> <endpoint>\n"; return 2; }
            auto r = nodes.RegisterPeer(argv[2], argv[3]);
            if (!r.Ok()) { std::cerr << "register failed\n"; return 1; }
            std::cout << "node_id=" << sovereign::node::NodeRegistry::IdHex(r.value) << "\n";
            return 0;
        }
        if (cmd == "node-list")
        {
            auto r = nodes.List();
            if (!r.Ok()) { std::cerr << "list failed\n"; return 1; }
            for (const auto& n : r.value)
            {
                std::cout << "name=" << n.name
                          << " kind=" << sovereign::node::KindName(n.kind)
                          << " health=" << sovereign::node::HealthName(n.health)
                          << " ws=" << n.workspace_name
                          << " caps=" << n.capabilities.size()
                          << "\n";
            }
            return 0;
        }
        if (cmd == "node-heartbeat")
        {
            if (argc < 3) { std::cerr << "usage: node-heartbeat <name>\n"; return 2; }
            auto f = nodes.FindByName(argv[2]);
            if (!f.Ok()) { std::cerr << "not found\n"; return 1; }
            auto r = nodes.Heartbeat(f.value.id);
            if (!r.Ok()) { std::cerr << "heartbeat failed\n"; return 1; }
            std::cout << "name=" << argv[2] << " health=healthy\n";
            return 0;
        }
        if (cmd == "node-bind")
        {
            if (argc < 4) { std::cerr << "usage: node-bind <name> <workspace>\n"; return 2; }
            auto f = nodes.FindByName(argv[2]);
            if (!f.Ok()) { std::cerr << "not found\n"; return 1; }
            auto r = nodes.BindWorkspace(f.value.id, argv[3]);
            if (!r.Ok()) { std::cerr << "bind failed\n"; return 1; }
            std::cout << "name=" << argv[2] << " workspace=" << argv[3] << "\n";
            return 0;
        }
        if (cmd == "node-caps")
        {
            if (argc < 4) { std::cerr << "usage: node-caps <name> <cap,cap,...>\n"; return 2; }
            auto f = nodes.FindByName(argv[2]);
            if (!f.Ok()) { std::cerr << "not found\n"; return 1; }
            std::vector<std::string> caps;
            std::string s = argv[3];
            std::string cur;
            for (char c : s) {
                if (c == ',') { if (!cur.empty()) caps.push_back(cur); cur.clear(); }
                else cur.push_back(c);
            }
            if (!cur.empty()) caps.push_back(cur);
            auto r = nodes.SetCapabilities(f.value.id, caps);
            if (!r.Ok()) { std::cerr << "caps failed\n"; return 1; }
            std::cout << "name=" << argv[2] << " caps=" << caps.size() << "\n";
            return 0;
        }
        if (cmd == "node-events")
        {
            auto r = nodes.RecentEvents(30);
            if (!r.Ok()) { std::cerr << "events failed\n"; return 1; }
            for (const auto& e : r.value) std::cout << e << "\n";
            return 0;
        }
        std::cerr << "unknown node command\n";
        return 2;
    }

    if (cmd == "capability-seed")
        {
            auto r = caps.SeedDefaults();
            if (!r.Ok()) { std::cerr << "seed failed\n"; return 1; }
            std::cout << "seeded_added=" << r.value << "\n";
            return 0;
        }
        if (cmd == "capability-list")
        {
            auto r = caps.List();
            if (!r.Ok()) { std::cerr << "list failed\n"; return 1; }
            if (r.value.empty())
            {
                auto s = caps.SeedDefaults();
                (void)s;
                r = caps.List();
            }
            for (const auto& c : r.value)
            {
                std::cout << "id=" << c.id
                          << " enabled=" << (c.enabled ? 1 : 0)
                          << " source=" << c.source
                          << " desc=" << c.description << "\n";
            }
            return 0;
        }
        if (argc < 3) {
            std::cerr << "usage: " << cmd << " <id>\n";
            return 2;
        }
        std::string id = argv[2];
        if (cmd == "capability-enable" || cmd == "capability-disable")
        {
            auto r = caps.SetEnabled(id, cmd == "capability-enable");
            if (!r.Ok()) { std::cerr << "set failed\n"; return 1; }
            std::cout << "id=" << id << " enabled=" << (cmd == "capability-enable" ? 1 : 0) << "\n";
            return 0;
        }
        auto r = caps.IsEnabled(id);
        if (!r.Ok()) { std::cerr << "check failed\n"; return 1; }
        std::cout << "id=" << id << " enabled=" << (r.value ? 1 : 0) << "\n";
        return 0;
    }

    if (cmd == "workspace-status")
        {
            if (argc >= 3)
            {
                auto r = resolve(argv[2]);
                if (!r.Ok()) { std::cerr << "workspace not found\n"; return 1; }
                std::cout << "name=" << r.value.name
                          << " kind=" << r.value.kind
                          << " status=" << sovereign::workspace::WorkspaceService::StatusName(r.value.status)
                          << "\n";
                return 0;
            }
            auto all = host.Workspace().List();
            if (!all.Ok()) { std::cerr << "list failed\n"; return 1; }
            for (const auto& w : all.value)
            {
                std::cout << "name=" << w.name
                          << " kind=" << w.kind
                          << " status=" << sovereign::workspace::WorkspaceService::StatusName(w.status)
                          << "\n";
            }
            return 0;
        }
        if (argc < 3) {
            std::cerr << "usage: " << cmd << " <name-or-hex>\n";
            return 2;
        }
        auto r = resolve(argv[2]);
        if (!r.Ok()) { std::cerr << "workspace not found\n"; return 1; }
        sovereign::workspace::Result<void> op;
        if (cmd == "workspace-activate")
            op = host.Workspace().Activate(r.value.id);
        else
            op = host.Workspace().Suspend(r.value.id);
        if (!op.Ok()) { std::cerr << cmd << " failed\n"; return 1; }
        auto again = host.Workspace().Get(r.value.id);
        std::cout << "name=" << r.value.name
                  << " status=" << sovereign::workspace::WorkspaceService::StatusName(again.value.status)
                  << "\n";
        return 0;
    }

    if (cmd == "workspace-list")
    {
        auto r = host.Workspace().List();
        if (!r.Ok())
        {
            host.Close();
            return 1;
        }
        for (const auto& w : r.value)
        {
            std::cout << ToHex(w.id.bytes) << ' ' << w.name << ' ' << w.kind
                      << ' ' << static_cast<int>(w.status) << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "memory-add" && argc >= 4)
    {
        sovereign::memory::MemoryRecord rec;
        rec.tags = {argv[2]};
        std::string body;
        for (int i = 3; i < argc; ++i)
        {
            if (i > 3)
                body.push_back(' ');
            body += argv[i];
        }
        rec.body = body;
        auto r = host.Memory().Append(rec);
        if (!r.Ok())
        {
            std::cerr << "memory-add failed\n";
            host.Close();
            return 1;
        }
        std::cout << "record_id=" << ToHex(r.value.bytes) << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "memory-list")
    {
        if (argc >= 3)
        {
            auto r = host.Memory().ListByTag(argv[2]);
            if (!r.Ok())
            {
                std::cerr << "memory-list failed\n";
                host.Close();
                return 1;
            }
            for (const auto& rec : r.value)
            {
                std::cout << ToHex(rec.id.bytes) << ' ' << rec.body << '\n';
            }
        }
        else
        {
            auto r = host.Memory().ListAll();
            if (!r.Ok())
            {
                std::cerr << "memory-list failed\n";
                host.Close();
                return 1;
            }
            for (const auto& rec : r.value)
            {
                std::cout << ToHex(rec.id.bytes) << ' ';
                for (std::size_t i = 0; i < rec.tags.size(); ++i)
                {
                    if (i)
                        std::cout << ',';
                    std::cout << rec.tags[i];
                }
                std::cout << ' ' << rec.body << '\n';
            }
        }
        host.Close();
        return 0;
    }

    if (cmd == "policy-check" && argc >= 4)
    {
        sovereign::PrincipalId id{};
        std::vector<std::string> roles{argv[2]};
        auto r = host.Policy().Evaluate(id, roles, argv[3]);
        if (!r.Ok())
        {
            std::cerr << "policy-check failed\n";
            host.Close();
            return 1;
        }
        std::cout << "effect="
                  << (r.value.effect == sovereign::policy::PolicyEffect::Allow
                          ? "allow"
                          : "deny")
                  << " reason=" << r.value.reason
                  << " version=" << r.value.policy_version
                  << " dry_run=" << (r.value.dry_run ? "1" : "0") << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "policy-dry-run" && argc >= 4)
    {
        sovereign::PrincipalId id{};
        std::vector<std::string> roles{argv[2]};
        auto r = host.Policy().DryRun(id, roles, argv[3]);
        if (!r.Ok())
        {
            std::cerr << "policy-dry-run failed\n";
            host.Close();
            return 1;
        }
        std::cout << "effect="
                  << (r.value.effect == sovereign::policy::PolicyEffect::Allow
                          ? "allow"
                          : "deny")
                  << " reason=" << r.value.reason
                  << " version=" << r.value.policy_version
                  << " dry_run=1\n";
        host.Close();
        return 0;
    }

    if (cmd == "policy-version")
    {
        std::cout << "policy_version=" << host.Policy().Version() << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "policy-rule-set" && argc >= 5)
    {
        sovereign::policy::PolicyRule rule;
        rule.action = argv[2];
        rule.min_role = argv[3];
        rule.effect = (std::string(argv[4]) == "allow")
                          ? sovereign::policy::PolicyEffect::Allow
                          : sovereign::policy::PolicyEffect::Deny;
        auto r = host.Policy().SetRule(rule);
        if (!r.Ok())
        {
            std::cerr << "policy-rule-set failed\n";
            host.Close();
            return 1;
        }
        std::cout << "policy_version=" << host.Policy().Version() << '\n';
        {
            sovereign::audit::AuditEntry e;
            e.category = "policy";
            e.action = "rule_set";
            e.subject = argv[2];
            e.detail = std::string(argv[3]) + " " + argv[4];
            static_cast<void>(host.Audit().Append(e));
        }
        host.Close();
        return 0;
    }

    
    if (cmd == "secret-put" || cmd == "secret-get" || cmd == "secret-list"
        || cmd == "secret-exists" || cmd == "secret-delete")
    {
        sovereign::secrets::SecretManager secrets;
        if (secrets.Open(host.DataRoot()) != sovereign::secrets::SecretError::Ok)
        {
            std::cerr << "secrets open failed\n";
            return 1;
        }
        secrets.SetAuditHook([&](const std::string& action, const std::string& name) {
            sovereign::audit::AuditEntry e;
            e.category = "secrets";
            e.action = action;
            e.subject = name;
            e.detail = "secret_manager";
            (void)host.Audit().Append(e);
        });
        if (cmd == "secret-put")
        {
            if (argc < 4)
            {
                std::cerr << "usage: secret-put <name> <value>\n";
                return 2;
            }
            auto r = secrets.Put(argv[2], argv[3]);
            if (!r.Ok())
            {
                std::cerr << "secret-put failed\n";
                return 1;
            }
            std::cout << "secret_stored name=" << argv[2] << "\n";
            return 0;
        }
        if (cmd == "secret-get")
        {
            if (argc < 3)
            {
                std::cerr << "usage: secret-get <name>\n";
                return 2;
            }
            auto r = secrets.Get(argv[2]);
            if (!r.Ok())
            {
                std::cerr << "secret-get failed\n";
                return 1;
            }
            std::cout << r.value << "\n";
            return 0;
        }
        if (cmd == "secret-list")
        {
            auto r = secrets.List();
            if (!r.Ok())
            {
                std::cerr << "secret-list failed\n";
                return 1;
            }
            for (const auto& m : r.value)
            {
                std::cout << "name=" << m.name << " bytes=" << m.value_bytes
                          << " updated=" << m.updated_unix << "\n";
            }
            return 0;
        }
        if (cmd == "secret-exists")
        {
            if (argc < 3)
            {
                std::cerr << "usage: secret-exists <name>\n";
                return 2;
            }
            auto r = secrets.Exists(argv[2]);
            if (!r.Ok())
            {
                std::cerr << "secret-exists failed\n";
                return 1;
            }
            std::cout << "exists=" << (r.value ? 1 : 0) << "\n";
            return 0;
        }
        if (cmd == "secret-delete")
        {
            if (argc < 3)
            {
                std::cerr << "usage: secret-delete <name>\n";
                return 2;
            }
            auto r = secrets.Delete(argv[2]);
            if (!r.Ok())
            {
                std::cerr << "secret-delete failed\n";
                return 1;
            }
            std::cout << "secret_deleted name=" << argv[2] << "\n";
            return 0;
        }
    }

    if (cmd == "audit-verify")
    {
        auto r = host.Audit().VerifyChain();
        if (!r.Ok())
        {
            std::cerr << "audit-verify failed\n";
            host.Close();
            return 1;
        }
        auto tip = host.Audit().ChainTip();
        std::cout << "audit_chain_ok=" << (r.value ? "1" : "0")
                  << " tip=" << (tip.Ok() ? tip.value : "?") << '\n';
        host.Close();
        return 0;
    }

    
    
    if (cmd == "memory-facet-add" || cmd == "memory-facet-list" || cmd == "memory-facet-counts")
    {
        if (cmd == "memory-facet-counts")
        {
            auto r = host.Memory().CountByFacet();
            if (!r.Ok())
            {
                std::cerr << "memory-facet-counts failed\n";
                return 1;
            }
            for (const auto& c : r.value)
                std::cout << "facet=" << c.facet << " count=" << c.count << "\n";
            return 0;
        }
        if (cmd == "memory-facet-list")
        {
            std::string facet = (argc >= 3) ? argv[2] : "";
            if (facet.empty())
            {
                auto all = host.Memory().ListAll();
                if (!all.Ok())
                {
                    std::cerr << "memory-facet-list failed\n";
                    return 1;
                }
                for (const auto& rec : all.value)
                {
                    auto f = sovereign::memory::FacetFromTags(rec.tags);
                    if (f.empty())
                        continue;
                    std::cout << "facet=" << f << " body=" << rec.body.substr(0, 120) << "\n";
                }
                return 0;
            }
            auto r = host.Memory().ListByFacet(facet);
            if (!r.Ok())
            {
                std::cerr << "memory-facet-list failed (unknown facet?)\n";
                return 1;
            }
            for (const auto& rec : r.value)
                std::cout << "body=" << rec.body << "\n";
            return 0;
        }
        if (cmd == "memory-facet-add")
        {
            if (argc < 5)
            {
                std::cerr << "usage: memory-facet-add <facet> <tag> <body...>\n";
                std::cerr << "facets: constitutional working decision operational knowledge execution\n";
                return 2;
            }
            std::string facet = argv[2];
            if (!sovereign::memory::IsKnownFacet(facet))
            {
                std::cerr << "unknown facet\n";
                return 2;
            }
            std::string tag = argv[3];
            std::string body;
            for (int i = 4; i < argc; ++i)
            {
                if (!body.empty())
                    body.push_back(' ');
                body += argv[i];
            }
            sovereign::memory::MemoryRecord rec;
            rec.tags.push_back(sovereign::memory::FacetTag(facet));
            rec.tags.push_back(tag);
            rec.body = body;
            auto r = host.Memory().Append(rec);
            if (!r.Ok())
            {
                std::cerr << "memory-facet-add failed\n";
                return 1;
            }
            std::cout << "facet=" << facet << " record stored\n";
            return 0;
        }
    }

    if (cmd == "memory-rank")
    {
        auto r = host.Memory().Ranked(20);
        if (!r.Ok())
        {
            std::cerr << "memory-rank failed\n";
            host.Close();
            return 1;
        }
        for (const auto& s : r.value)
        {
            std::cout << "score=" << s.score << " importance=" << s.record.importance
                      << " body=" << s.record.body.substr(0, 80) << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "memory-prune" && argc >= 3)
    {
        double min_score = std::stod(argv[2]);
        auto r = host.Memory().PruneBelow(min_score);
        if (!r.Ok())
        {
            std::cerr << "memory-prune failed\n";
            host.Close();
            return 1;
        }
        std::cout << "removed=" << r.value << '\n';
        {
            sovereign::audit::AuditEntry e;
            e.category = "memory";
            e.action = "prune";
            e.detail = std::string("removed=") + std::to_string(r.value);
            static_cast<void>(host.Audit().Append(e));
        }
        host.Close();
        return 0;
    }

    if (cmd == "memory-importance" && argc >= 4)
    {
        sovereign::memory::RecordId id{};
        // accept hex id
        auto hex = std::string(argv[2]);
        if (hex.size() == 32)
        {
            auto n = [](char c) {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
            };
            for (std::size_t i = 0; i < 16; ++i)
            {
                int hi = n(hex[i * 2]), lo = n(hex[i * 2 + 1]);
                if (hi < 0 || lo < 0)
                {
                    std::cerr << "bad record id\n";
                    host.Close();
                    return 1;
                }
                id.bytes[i] = static_cast<std::uint8_t>((hi << 4) | lo);
            }
        }
        double imp = std::stod(argv[3]);
        auto r = host.Memory().SetImportance(id, imp);
        if (!r.Ok())
        {
            std::cerr << "memory-importance failed\n";
            host.Close();
            return 1;
        }
        std::cout << "ok\n";
        host.Close();
        return 0;
    }

    if (cmd == "curious" && argc >= 3)
    {
        std::string topic;
        for (int i = 2; i < argc; ++i)
        {
            if (i > 2)
                topic.push_back(' ');
            topic += argv[i];
        }

        sovereign::PrincipalId actor{};
        auto proposal = host.Curiosity().Propose(actor, topic, {});
        if (!proposal.Ok())
        {
            std::cerr << "curious failed\n";
            host.Close();
            return 1;
        }

        std::cout << "curiosity topic=" << proposal.value.topic << '\n';
        std::cout << "questions=" << proposal.value.questions.size() << '\n';
        for (std::size_t i = 0; i < proposal.value.questions.size(); ++i)
        {
            std::cout << "q" << i + 1 << ": " << proposal.value.questions[i] << '\n';
        }

        const auto queue = host.Curiosity().DiscoveryQueueSnapshot();
        std::cout << "queue_size=" << queue.size() << '\n';
        if (!queue.empty())
        {
            std::cout << "next_queue=" << queue.front().topic << " priority=" << queue.front().priority << '\n';
        }

        const auto graph = host.Curiosity().InterestGraph(3);
        std::cout << "interest_top=" << graph.size() << '\n';
        for (std::size_t i = 0; i < graph.size(); ++i)
        {
            std::cout << "interest" << i + 1 << "=" << graph[i].topic
                      << " score=" << graph[i].score << " mentions=" << graph[i].mentions << '\n';
        }

        sovereign::audit::AuditEntry e;
        e.category = "cognitive";
        e.action = "curiosity";
        e.detail = topic;
        e.subject = std::to_string(proposal.value.questions.size());
        static_cast<void>(host.Audit().Append(e));

        host.Close();
        return 0;
    }

    if (cmd == "evolve")
    {
        std::filesystem::path root = std::filesystem::current_path();
        std::size_t limit = 5;

        for (int i = 2; i < argc; ++i)
        {
            const std::string arg = argv[i];
            if (arg == "--limit" && i + 1 < argc)
            {
                limit = static_cast<std::size_t>(std::stoul(argv[++i]));
                continue;
            }
            root = std::filesystem::path(arg);
        }

        auto candidates = host.Evolution().ScanSystem(root, limit);
        if (candidates.empty())
        {
            std::cout << "no_evolution_candidates\n";
            host.Close();
            return 0;
        }

        std::cout << "evolution_candidates=" << candidates.size() << '\n';
        for (std::size_t i = 0; i < candidates.size(); ++i)
        {
            const auto& c = candidates[i];
            std::cout << "candidate" << i + 1 << " subsystem=" << c.subsystem
                      << " priority=" << c.priority << " path=" << c.file_path << '\n';
            std::cout << "  title=" << c.title << '\n';
            std::cout << "  summary=" << c.summary << '\n';
        }

        sovereign::audit::AuditEntry e;
        e.category = "cognitive";
        e.action = "evolution_scan";
        e.detail = root.string();
        e.subject = std::to_string(candidates.size());
        static_cast<void>(host.Audit().Append(e));

        host.Close();
        return 0;
    }

    if (cmd == "sandbox-propose")
    {
        std::filesystem::path root = std::filesystem::current_path();
        std::size_t limit = 5;
        for (int i = 2; i < argc; ++i)
        {
            const std::string arg = argv[i];
            if (arg == "--limit" && i + 1 < argc)
            {
                limit = static_cast<std::size_t>(std::stoul(argv[++i]));
                continue;
            }
            root = std::filesystem::path(arg);
        }

        auto sandbox = host.SandboxEvolution();
        auto proposal = sandbox.CreateProposal(limit);
        std::cout << "proposal_id=" << proposal.id << '\n';
        std::cout << "status=" << static_cast<int>(proposal.status) << '\n';
        std::cout << "sandbox_root=" << proposal.sandbox_root.string() << '\n';
        std::cout << "title=" << proposal.title << '\n';
        std::cout << "summary=" << proposal.summary << '\n';
        std::cout << "validation_command=" << proposal.validation_command << '\n';

        sovereign::audit::AuditEntry e;
        e.category = "cognitive";
        e.action = "sandbox_proposal_created";
        e.detail = proposal.title;
        e.subject = proposal.id;
        static_cast<void>(host.Audit().Append(e));

        host.Close();
        return 0;
    }

    if (cmd == "sandbox-approve")
    {
        if (argc < 4)
        {
            std::cerr << "usage: sovereign-cli sandbox-approve <proposal-id> <approver>\n";
            host.Close();
            return 1;
        }

        const std::string proposal_id = argv[2];
        const std::string approver = argv[3];
        auto sandbox = host.SandboxEvolution();
        auto proposals = sandbox.PendingProposals();
        const auto it = std::find_if(
            proposals.begin(), proposals.end(),
            [&](const auto& item) { return item.id == proposal_id; });
        if (it == proposals.end())
        {
            std::cerr << "sandbox proposal not found: " << proposal_id << '\n';
            host.Close();
            return 1;
        }

        const auto proposal = *it;
        if (!sandbox.ValidateProposal(proposal) || !sandbox.ApproveProposal(proposal_id, approver))
        {
            std::cerr << "sandbox approval failed for proposal " << proposal_id << '\n';
            host.Close();
            return 1;
        }

        std::cout << "sandbox_approved proposal_id=" << proposal_id << " approver=" << approver << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "external-request" && argc >= 4)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto session = host.Identity().ValidateSession(sid);
        if (!session.Ok())
        {
            std::cerr << "session invalid; run login again\n";
            host.Close();
            return 1;
        }

        std::string action = argv[2];
        std::string summary;
        for (int i = 3; i < argc; ++i)
        {
            if (i > 3)
                summary.push_back(' ');
            summary += argv[i];
        }

        std::string decision_hex;
        if (!host.External().Request(session.value.principal_id, action, summary, "cli:external-request", decision_hex))
        {
            std::cerr << "external request failed\n";
            host.Close();
            return 1;
        }

        std::cout << "decision_hex=" << decision_hex << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "external-approve" && argc >= 4)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto session = host.Identity().ValidateSession(sid);
        if (!session.Ok())
        {
            std::cerr << "session invalid; run login again\n";
            host.Close();
            return 1;
        }

        std::string reason;
        for (int i = 3; i < argc; ++i)
        {
            if (i > 3)
                reason.push_back(' ');
            reason += argv[i];
        }

        if (!host.External().Approve(session.value.principal_id, argv[2], reason))
        {
            std::cerr << "external approval failed\n";
            host.Close();
            return 1;
        }

        std::cout << "external approved\n";
        host.Close();
        return 0;
    }

    if (cmd == "plan" && argc >= 3)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::PrincipalId actor{};
        // use session validation path via identity if available — store principal in session file?
        // Fallback: nil principal is ok for plan heuristics
        std::string goal;
        for (int i = 2; i < argc; ++i)
        {
            if (i > 2) goal.push_back(' ');
            goal += argv[i];
        }
        std::cout << "plan: building..." << std::endl;
        auto r = host.Cognitive().BuildPlan(actor, goal);
        if (!r.Ok())
        {
            std::cerr << "plan failed\n";
            host.Close();
            return 1;
        }
        std::cout << "goal=" << r.value.goal << std::endl;
        std::cout << "confidence=" << r.value.confidence << std::endl;
        std::cout << "enrichment=" << r.value.enrichment_status << std::endl;
        {
            sovereign::audit::AuditEntry e;
            e.category = "cognitive";
            e.action = "plan";
            e.detail = r.value.goal;
            e.subject = r.value.enrichment_status;
            static_cast<void>(host.Audit().Append(e));
        }
        std::cout << "uncertainty=" << r.value.uncertainty_notes << '\n';
        for (std::size_t i = 0; i < r.value.steps.size(); ++i)
        {
            const auto& st = r.value.steps[i];
            std::cout << "step" << i << " action=" << st.action
                      << " gov=" << (st.requires_governance ? "1" : "0")
                      << " detail=" << st.detail << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "audit-list")
    {
        if (argc >= 3)
        {
            auto r = host.Audit().ListByCategory(argv[2], 50);
            if (!r.Ok())
            {
                std::cerr << "audit-list failed\n";
                host.Close();
                return 1;
            }
            for (const auto& e : r.value)
            {
                std::cout << e.unix_seconds << ' ' << e.category << ' '
                          << e.action << ' ' << e.subject << ' ' << e.detail
                          << '\n';
            }
        }
        else
        {
            auto r = host.Audit().List(50);
            if (!r.Ok())
            {
                std::cerr << "audit-list failed\n";
                host.Close();
                return 1;
            }
            for (const auto& e : r.value)
            {
                std::cout << e.unix_seconds << ' ' << e.category << ' '
                          << e.action << ' ' << e.subject << ' ' << e.detail
                          << '\n';
            }
        }
        host.Close();
        return 0;
    }


    if (cmd == "loop" && argc >= 3)
    {
        // Governed cognitive loop: plan → recommend → [approve] → execute → reflect
        // Usage: sovereign-cli loop [--auto-approve] <goal...>
        bool auto_approve = false;
        int argi = 2;
        if (argi < argc && std::string(argv[argi]) == "--auto-approve")
        {
            auto_approve = true;
            ++argi;
        }
        if (argi >= argc)
        {
            std::cerr << "usage: sovereign-cli loop [--auto-approve] <goal...>\n";
            host.Close();
            return 1;
        }
        std::string goal;
        for (int i = argi; i < argc; ++i)
        {
            if (i > argi) goal.push_back(' ');
            goal += argv[i];
        }

        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto session = host.Identity().ValidateSession(sid);
        if (!session.Ok())
        {
            std::cerr << "session invalid; run login again\n";
            host.Close();
            return 1;
        }
        const sovereign::PrincipalId actor = session.value.principal_id;

        std::cout << "=== loop: plan ===\n";
        auto plan = host.Cognitive().BuildPlan(actor, goal);
        if (!plan.Ok())
        {
            std::cerr << "plan failed\n";
            host.Close();
            return 1;
        }
        std::cout << "confidence=" << plan.value.confidence << '\n';
        std::cout << "enrichment=" << plan.value.enrichment_status << '\n';
        for (std::size_t i = 0; i < plan.value.steps.size(); ++i)
            std::cout << "step" << i << " " << plan.value.steps[i].action
                      << " gov=" << (plan.value.steps[i].requires_governance ? 1 : 0)
                      << " " << plan.value.steps[i].detail << '\n';

        const std::string action = "mature";
        const std::string summary = goal;
        std::cout << "=== loop: recommend ===\n";
        auto rec = host.Cognitive().RecommendAction(actor, action, summary, "loop");
        if (!rec.Ok())
        {
            std::cerr << "recommend failed\n";
            host.Close();
            return 1;
        }
        const auto decision_hex = ToHex(rec.value.governance_decision_id);
        std::cout << "governance_decision_id=" << decision_hex << '\n';

        if (!auto_approve)
        {
            std::cout << "=== loop: paused for human approval ===\n";
            std::cout << "next:\n  sovereign-cli approve " << decision_hex
                      << " \"approved via loop\"\n  sovereign-cli execute " << action
                      << " " << decision_hex << "\n";
            sovereign::audit::AuditEntry e;
            e.category = "cognitive";
            e.action = "loop_paused";
            e.actor = actor;
            e.subject = decision_hex;
            e.detail = goal;
            static_cast<void>(host.Audit().Append(e));
            host.Close();
            return 0;
        }

        std::cout << "=== loop: auto-approve ===\n";
        sovereign::governance::DecisionId did{};
        did.bytes = rec.value.governance_decision_id;
        auto ap = host.Governance().Approve(actor, true, did, "loop --auto-approve");
        if (!ap.Ok())
        {
            std::cerr << "approve failed error=" << static_cast<int>(ap.error) << '\n';
            host.Close();
            return 1;
        }
        std::cout << "approved\n";

        std::cout << "=== loop: execute ===\n";
        host.Executor().RegisterHandler(action, [](const std::string& a) {
            sovereign::executor::Result<std::string> r;
            r.value = std::string("loop-executed:") + a;
            return r;
        });
        auto ex = host.Executor().Execute(action, rec.value.governance_decision_id);
        if (!ex.Ok())
        {
            std::cerr << "execute failed error=" << static_cast<int>(ex.error) << '\n';
            host.Close();
            return 1;
        }
        std::cout << "executed ok detail=" << ex.value.detail << '\n';

        std::cout << "=== loop: reflect ===\n";
        auto ref = host.Cognitive().Reflect(
            actor, goal, std::string("completed loop action=") + action +
                             " detail=" + ex.value.detail);
        if (ref.Ok())
            std::cout << "reflection recorded\n";
        else
            std::cout << "reflection skipped\n";

        sovereign::audit::AuditEntry e;
        e.category = "cognitive";
        e.action = "loop_complete";
        e.actor = actor;
        e.subject = decision_hex;
        e.detail = goal;
        static_cast<void>(host.Audit().Append(e));
        host.Close();
        return 0;
    }

    if (cmd == "recommend" && argc >= 4)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto session = host.Identity().ValidateSession(sid);
        if (!session.Ok())
        {
            std::cerr << "session invalid; run login again\n";
            host.Close();
            return 1;
        }
        sovereign::PrincipalId actor = session.value.principal_id;
        auto r = host.Cognitive().RecommendAction(
            actor, argv[2], argv[3], "cli");
        if (!r.Ok())
        {
            std::cerr << "recommend failed\n";
            host.Close();
            return 1;
        }
        std::cout << "governance_decision_id="
                  << ToHex(r.value.governance_decision_id) << '\n';
        host.Close();
        return 0;
    }


    if (cmd == "pending-decisions")
    {
        auto r = host.Governance().ListPending();
        if (!r.Ok())
        {
            std::cerr << "pending-decisions failed\n";
            host.Close();
            return 1;
        }
        for (const auto& d : r.value)
        {
            std::cout << ToHex(d.id.bytes) << ' ' << d.action << ' '
                      << d.summary << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "archive-store" && argc >= 4)
    {
        sovereign::archive::Artifact a;
        a.label = argv[2];
        a.content_hash = argv[3];
        auto r = host.Archive().Store(a);
        if (!r.Ok())
        {
            std::cerr << "archive-store failed\n";
            host.Close();
            return 1;
        }
        std::cout << "artifact_id=" << ToHex(r.value.bytes) << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "sync-export" && argc >= 4)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        sovereign::PrincipalId actor{};
        if (LoadSession(cfg.data_root, session_bytes))
        {
            sovereign::identity::SessionId sid{};
            sid.bytes = session_bytes;
            auto session = host.Identity().ValidateSession(sid);
            if (session.Ok())
                actor = session.value.principal_id;
        }
        auto r = host.Sync().RecordExport(actor, argv[2], argv[3]);
        if (!r.Ok())
        {
            std::cerr << "sync-export failed\n";
            host.Close();
            return 1;
        }
        std::cout << "sync_id=" << ToHex(r.value.bytes) << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "approve" && argc >= 4)
    {
        sovereign::governance::DecisionId id{};
        if (!FromHex(argv[2], id.bytes))
        {
            std::cerr << "bad decision id\n";
            host.Close();
            return 1;
        }
        sovereign::governance::PrincipalId owner{};
        owner.bytes[0] = 1;
        auto r = host.Governance().Approve(owner, true, id, argv[3]);
        if (!r.Ok())
        {
            std::cerr << "approve failed\n";
            host.Close();
            return 1;
        }
        std::cout << "approved\n";
        {
            sovereign::audit::AuditEntry e;
            e.category = "governance";
            e.action = "approve";
            e.subject = argv[2];
            e.detail = argv[3];
            static_cast<void>(host.Audit().Append(e));
            // Evidence pack: durable archive + memory + semantic
            sovereign::archive::Artifact a;
            a.label = std::string("governance-approve-") + argv[2];
            a.content_hash = argv[2];
            auto art = host.Archive().Store(a);
            if (art.Ok())
                std::cout << "evidence_artifact=" << ToHex(art.value.bytes)
                          << '\n';
            sovereign::memory::MemoryRecord rec;
            rec.tags = {"governance", "approve", "evidence"};
            rec.body = std::string("approved ") + argv[2] + " reason=" + argv[3];
            static_cast<void>(host.Memory().Append(rec));
            static_cast<void>(host.Semantic().Upsert(rec.body, rec.tags));
        }
        host.Close();
        return 0;
    }

    if (cmd == "reject" && argc >= 4)
    {
        sovereign::governance::DecisionId id{};
        if (!FromHex(argv[2], id.bytes))
        {
            std::cerr << "bad decision id\n";
            host.Close();
            return 1;
        }
        sovereign::governance::PrincipalId owner{};
        owner.bytes[0] = 1;
        auto r = host.Governance().Reject(owner, true, id, argv[3]);
        if (!r.Ok())
        {
            std::cerr << "reject failed\n";
            host.Close();
            return 1;
        }
        std::cout << "rejected\n";
        {
            sovereign::audit::AuditEntry e;
            e.category = "governance";
            e.action = "reject";
            e.subject = argv[2];
            e.detail = argv[3];
            static_cast<void>(host.Audit().Append(e));
        }
        host.Close();
        return 0;
    }

    if (cmd == "override" && argc >= 4)
    {
        sovereign::governance::DecisionId id{};
        if (!FromHex(argv[2], id.bytes))
        {
            std::cerr << "bad decision id\n";
            host.Close();
            return 1;
        }
        sovereign::governance::PrincipalId owner{};
        owner.bytes[0] = 1;
        auto r = host.Governance().Override(
            owner, sovereign::governance::GovernanceRole::Owner, id, argv[3]);
        if (!r.Ok())
        {
            std::cerr << "override failed\n";
            host.Close();
            return 1;
        }
        std::cout << "overridden\n";
        {
            sovereign::audit::AuditEntry e;
            e.category = "governance";
            e.action = "override";
            e.subject = argv[2];
            e.detail = argv[3];
            static_cast<void>(host.Audit().Append(e));
        }
        host.Close();
        return 0;
    }

    if (cmd == "rule-set" && argc >= 5)
    {
        sovereign::governance::GovernanceRule rule;
        rule.action = argv[2];
        rule.allow_auto_approval = std::string(argv[3]) == "1";
        rule.minimum_auto_approve_role =
            sovereign::governance::RoleFromString(argv[4]);
        rule.minimum_approval_authority_role =
            sovereign::governance::RoleFromString(argv[4]);
        host.Governance().SetRule(rule);
        std::cout << "rule_set action=" << rule.action << '\n';
        host.Close();
        return 0;
    }




    if (cmd == "mfa-enroll-begin")
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto r = host.Identity().BeginTotpEnrollment(sid);
        if (!r.Ok())
        {
            std::cerr << "mfa-enroll-begin failed error="
                      << static_cast<int>(r.error) << '\n';
            host.Close();
            return 1;
        }
        std::cout << "otpauth_uri=" << r.value << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "mfa-enroll-confirm" && argc >= 3)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto r = host.Identity().ConfirmTotpEnrollment(sid, argv[2]);
        if (!r.Ok())
        {
            PrintIdentityFailure("mfa-enroll-confirm", r);
            host.Close();
            return 1;
        }
        std::cout << "totp_enrolled=1\n";
        host.Close();
        return 0;
    }


    if (cmd == "mfa-recovery-generate")
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto r = host.Identity().GenerateRecoveryCodes(sid);
        if (!r.Ok())
        {
            PrintIdentityFailure("mfa-recovery-generate", r);
            host.Close();
            return 1;
        }
        for (const auto& c : r.value)
            std::cout << "recovery_code=" << c << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "mfa-status")
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto session = host.Identity().ValidateSession(sid);
        if (!session.Ok())
        {
            std::cerr << "session invalid\n";
            host.Close();
            return 1;
        }
        auto st = host.Identity().GetMfaStatus(session.value.principal_id);
        if (!st.Ok())
        {
            std::cerr << "mfa-status failed\n";
            host.Close();
            return 1;
        }
        std::cout << "totp_enrolled=" << (st.value.totp_enrolled ? "1" : "0")
                  << " step_up_execute="
                  << (st.value.step_up_required_for_execute ? "1" : "0")
                  << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "session-revoke")
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto r = host.Identity().RevokeSession(sid);
        if (!r.Ok())
        {
            PrintIdentityFailure("session-revoke", r);
            host.Close();
            return 1;
        }
        // clear local session file
        std::error_code ec;
        std::filesystem::remove(SessionFilePath(cfg.data_root), ec);
        sovereign::audit::AuditEntry e;
        e.category = "identity";
        e.action = "session_revoke";
        e.detail = "self-revoke via cli";
        static_cast<void>(host.Audit().Append(e));
        std::cout << "session_revoked\n";
        host.Close();
        return 0;
    }

    if (cmd == "principal-suspend" && argc >= 3)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        sovereign::PrincipalId id{};
        if (!FromHex(argv[2], id.bytes))
        {
            std::cerr << "bad principal id\n";
            host.Close();
            return 1;
        }
        auto r = host.Identity().SetPrincipalStatus(
            sid, id, sovereign::identity::PrincipalStatus::Suspended);
        if (!r.Ok())
        {
            PrintIdentityFailure("principal-suspend", r);
            host.Close();
            return 1;
        }
        sovereign::audit::AuditEntry e;
        e.category = "identity";
        e.action = "principal_suspend";
        e.subject = argv[2];
        e.detail = "suspended via cli";
        static_cast<void>(host.Audit().Append(e));
        std::cout << "principal_suspended id=" << argv[2] << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "principal-revoke" && argc >= 3)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        sovereign::PrincipalId id{};
        if (!FromHex(argv[2], id.bytes))
        {
            std::cerr << "bad principal id\n";
            host.Close();
            return 1;
        }
        auto r = host.Identity().SetPrincipalStatus(
            sid, id, sovereign::identity::PrincipalStatus::Revoked);
        if (!r.Ok())
        {
            PrintIdentityFailure("principal-revoke", r);
            host.Close();
            return 1;
        }
        sovereign::audit::AuditEntry e;
        e.category = "identity";
        e.action = "principal_revoke";
        e.subject = argv[2];
        e.detail = "revoked via cli";
        static_cast<void>(host.Audit().Append(e));
        std::cout << "principal_revoked id=" << argv[2] << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "logout")
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (LoadSession(cfg.data_root, session_bytes))
        {
            sovereign::identity::SessionId sid{};
            sid.bytes = session_bytes;
            static_cast<void>(host.Identity().RevokeSession(sid));
            std::error_code ec;
            std::filesystem::remove(SessionFilePath(cfg.data_root), ec);
        }
        {
            sovereign::audit::AuditEntry e;
            e.category = "identity";
            e.action = "logout";
            e.detail = "cli logout";
            static_cast<void>(host.Audit().Append(e));
        }
        std::cout << "logged_out\n";
        host.Close();
        return 0;
    }

    if (cmd == "operator-create" && argc >= 4)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto r = host.Identity().CreateOperator(sid, argv[2], argv[3]);
        if (!r.Ok())
        {
            PrintIdentityFailure("operator-create", r);
            host.Close();
            return 1;
        }
        std::cout << "operator_id=" << ToHex(r.value.bytes) << '\n';
        host.Close();
        return 0;
    }


    if (cmd == "service-create" && argc >= 3)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto r = host.Identity().CreateServicePrincipal(sid, argv[2]);
        if (!r.Ok())
        {
            PrintIdentityFailure("service-create", r);
            host.Close();
            return 1;
        }
        std::cout << "service_id=" << ToHex(r.value.bytes) << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "machine-create" && argc >= 3)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto r = host.Identity().CreateMachinePrincipal(sid, argv[2]);
        if (!r.Ok())
        {
            PrintIdentityFailure("machine-create", r);
            host.Close();
            return 1;
        }
        std::cout << "machine_id=" << ToHex(r.value.bytes) << '\n';
        host.Close();
        return 0;
    }

    if ((cmd == "role-add" || cmd == "role-grant") && argc >= 4)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        sovereign::PrincipalId id{};
        if (!FromHex(argv[2], id.bytes))
        {
            std::cerr << "bad principal id\n";
            host.Close();
            return 1;
        }

        const auto principal = host.Identity().GetPrincipal(id);
        if (!principal.Ok())
        {
            std::cerr << "role-add failed: "
                      << sovereign::identity::IdentityErrorToString(principal.error)
                      << '\n';
            host.Close();
            return 1;
        }

        auto r = host.Identity().AssignRole(sid, id, argv[3]);
        if (!r.Ok())
        {
            std::cerr << "role-add failed: "
                      << sovereign::identity::IdentityErrorToString(r.error)
                      << '\n';
            host.Close();
            return 1;
        }
        sovereign::audit::AuditEntry e;
        e.category = "identity";
        e.action = "role_add";
        e.subject = argv[2];
        e.detail = std::string("assigned role ") + argv[3];
        static_cast<void>(host.Audit().Append(e));
        std::cout << "role_added id=" << argv[2] << " role=" << argv[3] << '\n';
        host.Close();
        return 0;
    }

    if ((cmd == "role-remove" || cmd == "role-revoke") && argc >= 4)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        sovereign::PrincipalId id{};
        if (!FromHex(argv[2], id.bytes))
        {
            std::cerr << "bad principal id\n";
            host.Close();
            return 1;
        }

        const auto principal = host.Identity().GetPrincipal(id);
        if (!principal.Ok())
        {
            std::cerr << "role-remove failed: "
                      << sovereign::identity::IdentityErrorToString(principal.error)
                      << '\n';
            host.Close();
            return 1;
        }

        auto r = host.Identity().RevokeRole(sid, id, argv[3]);
        if (!r.Ok())
        {
            std::cerr << "role-remove failed: "
                      << sovereign::identity::IdentityErrorToString(r.error)
                      << '\n';
            host.Close();
            return 1;
        }
        sovereign::audit::AuditEntry e;
        e.category = "identity";
        e.action = "role_remove";
        e.subject = argv[2];
        e.detail = std::string("revoked role ") + argv[3];
        static_cast<void>(host.Audit().Append(e));
        std::cout << "role_removed id=" << argv[2] << " role=" << argv[3] << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "roles" && argc >= 3)
    {
        sovereign::PrincipalId id{};
        if (!FromHex(argv[2], id.bytes))
        {
            std::cerr << "bad principal id\n";
            host.Close();
            return 1;
        }

        const auto principal = host.Identity().GetPrincipal(id);
        if (!principal.Ok())
        {
            std::cerr << "roles failed: "
                      << sovereign::identity::IdentityErrorToString(principal.error)
                      << '\n';
            host.Close();
            return 1;
        }

        auto r = host.Identity().GetRoles(id);
        if (!r.Ok())
        {
            std::cerr << "roles failed: "
                      << sovereign::identity::IdentityErrorToString(r.error)
                      << '\n';
            host.Close();
            return 1;
        }
        for (const auto& role : r.value)
            std::cout << role << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "decision-get" && argc >= 3)
    {
        sovereign::governance::DecisionId id{};
        if (!FromHex(argv[2], id.bytes))
        {
            std::cerr << "bad decision id\n";
            host.Close();
            return 1;
        }
        auto r = host.Governance().Get(id);
        if (!r.Ok())
        {
            std::cerr << "decision-get failed\n";
            host.Close();
            return 1;
        }
        const auto& d = r.value;
        std::cout << "id=" << ToHex(d.id.bytes) << '\n';
        std::cout << "action=" << d.action << '\n';
        std::cout << "summary=" << d.summary << '\n';
        std::cout << "status=" << static_cast<int>(d.status) << '\n';
        std::cout << "evidence=" << d.evidence_ref << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "change-credential" && argc >= 4)
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto r = host.Identity().ChangeCredential(sid, argv[2], argv[3]);
        if (!r.Ok())
        {
            PrintIdentityFailure("change-credential", r);
            host.Close();
            return 1;
        }
        std::cout << "credential_changed\n";
        {
            sovereign::audit::AuditEntry e;
            e.category = "identity";
            e.action = "change_credential";
            e.detail = "credential rotated";
            static_cast<void>(host.Audit().Append(e));
        }
        host.Close();
        return 0;
    }

    if (cmd == "principals")
    {
        std::array<std::uint8_t, 16> session_bytes{};
        if (!LoadSession(cfg.data_root, session_bytes))
        {
            std::cerr << "no session; run login first\n";
            host.Close();
            return 1;
        }
        sovereign::identity::SessionId sid{};
        sid.bytes = session_bytes;
        auto r = host.Identity().ListPrincipals(sid);
        if (!r.Ok())
        {
            PrintIdentityFailure("principals", r);
            host.Close();
            return 1;
        }
        for (const auto& pr : r.value)
        {
            std::cout << ToHex(pr.id.bytes) << ' ' << pr.display_name << ' '
                      << static_cast<int>(pr.kind) << ' '
                      << static_cast<int>(pr.status) << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "models")
    {
        auto r = host.Models().ListModels();
        if (!r.Ok())
        {
            std::cerr << "models unavailable (is ollama running?)\n";
            host.Close();
            return 1;
        }
        for (const auto& m : r.value)
        {
            std::cout << m.name << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "code-assist" && argc >= 3)
    {
        std::vector<std::string> files;
        std::string prompt;
        bool saw_dd = false;
        bool json_out = false;
        bool save = false;
        for (int i = 2; i < argc; ++i)
        {
            const std::string a = argv[i];
            if (!saw_dd && a == "--")
            {
                saw_dd = true;
                continue;
            }
            if (!saw_dd && a == "--json")
            {
                json_out = true;
                continue;
            }
            if (!saw_dd && a == "--save")
            {
                save = true;
                continue;
            }
            if (!saw_dd && a == "--file" && i + 1 < argc)
            {
                files.push_back(argv[++i]);
                continue;
            }
            if (!saw_dd && a.rfind("--file=", 0) == 0)
            {
                files.push_back(a.substr(7));
                continue;
            }
            if (!prompt.empty())
                prompt.push_back(' ');
            prompt += a;
        }
        if (prompt.empty())
        {
            std::cerr << "code-assist requires a prompt\n";
            host.Close();
            return 2;
        }

        std::ostringstream user;
        user << prompt;
        constexpr std::size_t kMaxFileBytes = 32 * 1024;
        for (const auto& path : files)
        {
            std::ifstream in(path);
            if (!in)
            {
                user << "\n\n[file missing: " << path << "]";
                continue;
            }
            std::string body;
            char buf[4096];
            while (in && body.size() < kMaxFileBytes)
            {
                in.read(buf, sizeof(buf));
                body.append(buf, static_cast<std::size_t>(in.gcount()));
            }
            if (body.size() >= kMaxFileBytes)
                body += "\n...[truncated]";
            user << "\n\n----- FILE: " << path << " -----\n" << body;
        }

        sovereign::model_router::ChatRequest req;
        req.model = host.DefaultModel();
        std::string system =
            "You are Sovereign code-assist using openclaw. "
            "Answer concisely. No system changes. Cite file paths when relevant.";
        if (json_out)
        {
            system +=
                " Respond with a single JSON object only, keys: "
                "summary (string), findings (array of strings), "
                "suggested_fixes (array of strings). No markdown.";
        }
        req.messages.push_back(sovereign::model_router::ChatMessage{
            "system", system, {}, {}});
        req.messages.push_back(sovereign::model_router::ChatMessage{
            "user", user.str(), {}, {}});
        auto r = host.Models().Chat(req);
        if (!r.Ok() || !r.value.ok)
        {
            std::cerr << "code-assist failed\n";
            host.Close();
            return 1;
        }
        std::cout << r.value.content << '\n';
        if (save)
        {
            sovereign::memory::MemoryRecord rec;
            rec.tags = {"code-assist"};
            rec.body = r.value.content;
            auto app = host.Memory().Append(rec);
            if (app.Ok())
                std::cout << "saved_record_id=" << ToHex(app.value.bytes) << '\n';
        }
        host.Close();
        return 0;
    }

    if ((cmd == "chat" || cmd == "chat-tools") && argc >= 3)
    {
        std::string prompt;
        for (int i = 2; i < argc; ++i)
        {
            if (i > 2)
                prompt.push_back(' ');
            prompt += argv[i];
        }
        sovereign::model_router::ChatRequest req;
        req.model = host.DefaultModel();
        req.messages.push_back(
            sovereign::model_router::ChatMessage{"user", prompt, {}, {}});
        if (cmd == "chat-tools")
        {
            sovereign::model_router::ToolDefinition tool;
            tool.name = "record_memory";
            tool.description = "Store a short note in Sovereign memory";
            tool.parameters_json =
                "{\"type\":\"object\",\"properties\":{"
                "\"tag\":{\"type\":\"string\"},"
                "\"body\":{\"type\":\"string\"}},"
                "\"required\":[\"tag\",\"body\"]}";
            req.tools.push_back(tool);
        }
        auto r = host.Models().Chat(req);
        if (!r.Ok() || !r.value.ok)
        {
            std::cerr << "chat failed error=" << static_cast<int>(r.error)
                      << " detail=" << r.value.error << '\n';
            host.Close();
            return 1;
        }
        std::cout << "model=" << r.value.model << '\n';
        std::cout << "content=" << r.value.content << '\n';
        std::vector<sovereign::model_router::ChatMessage> follow =
            req.messages;
        follow.push_back(sovereign::model_router::ChatMessage{
            "assistant", r.value.content, {}, {}});
        bool any_tool = false;
        for (const auto& tc : r.value.tool_calls)
        {
            std::cout << "tool_call name=" << tc.name
                      << " args=" << tc.arguments_json << '\n';
            std::string tool_result = "ok";
            if (tc.name == "record_memory")
            {
                sovereign::memory::MemoryRecord rec;
                rec.tags = {"ollama-tool"};
                rec.body = tc.arguments_json.empty() ? prompt : tc.arguments_json;
                auto app = host.Memory().Append(rec);
                if (app.Ok())
                {
                    tool_result = std::string("stored:") + ToHex(app.value.bytes);
                    std::cout << "tool_result record_id="
                              << ToHex(app.value.bytes) << '\n';
                }
                else
                {
                    tool_result = "storage_failed";
                }
            }
            sovereign::model_router::ChatMessage tr;
            tr.role = "tool";
            tr.content = tool_result;
            tr.tool_name = tc.name;
            tr.tool_call_id = tc.id;
            follow.push_back(tr);
            any_tool = true;
        }
        if (any_tool)
        {
            sovereign::model_router::ChatRequest req2;
            req2.model = host.DefaultModel();
            req2.messages = follow;
            auto r2 = host.Models().Chat(req2);
            if (r2.Ok() && r2.value.ok)
            {
                std::cout << "followup_content=" << r2.value.content << '\n';
            }
        }
        host.Close();
        return 0;
    }

    if (cmd == "execute" && argc >= 4)
    {
        {
            sovereign::capability::CapabilityRegistry _caps;
            if (_caps.Open(host.DataRoot()) == sovereign::capability::CapabilityError::Ok) {
                (void)_caps.SeedDefaults();
                sovereign::capability::CapabilityGate _gate(_caps);
                auto _d = _gate.RequireForAction("execute");
                if (!_d.allowed) {
                    std::cerr << "capability_denied id=" << _d.capability_id
                              << " reason=" << _d.reason << "\n";
                    host.Close();
                    return 3;
                }
            }
        }

        std::array<std::uint8_t, 16> decision{};
        if (!FromHex(argv[3], decision))
        {
            std::cerr << "bad decision id\n";
            host.Close();
            return 1;
        }
        std::string mfa_code;
        for (int i = 4; i + 1 < argc; ++i)
        {
            if (std::string(argv[i]) == "--mfa")
                mfa_code = argv[i + 1];
        }
        // Step-up when MFA enrolled
        {
            std::array<std::uint8_t, 16> session_bytes{};
            if (LoadSession(cfg.data_root, session_bytes))
            {
                sovereign::identity::SessionId sid{};
                sid.bytes = session_bytes;
                auto session = host.Identity().ValidateSession(sid);
                if (session.Ok())
                {
                    auto st = host.Identity().GetMfaStatus(session.value.principal_id);
                    if (st.Ok() && st.value.step_up_required_for_execute)
                    {
                        if (mfa_code.empty())
                        {
                            std::cerr << "mfa_required: pass --mfa <totp-or-recovery>\n";
                            host.Close();
                            return 1;
                        }
                        auto su = host.Identity().VerifyStepUp(sid, mfa_code);
                        if (!su.Ok())
                        {
                            std::cerr << "mfa_step_up_failed error="
                                      << static_cast<int>(su.error) << '\n';
                            host.Close();
                            return 1;
                        }
                    }
                }
            }
        }
        host.Executor().RegisterHandler(argv[2], [](const std::string& action) {
            sovereign::executor::Result<std::string> r;
            r.value = std::string("executed:") + action;
            return r;
        });
        auto r = host.Executor().Execute(argv[2], decision);
        if (!r.Ok())
        {
            std::cerr << "execute failed error=" << static_cast<int>(r.error)
                      << '\n';
            host.Close();
            return 1;
        }
        std::cout << "executed ok detail=" << r.value.detail << '\n';
        {
            sovereign::audit::AuditEntry e;
            e.category = "executor";
            e.action = "execute";
            e.subject = argv[3];
            e.detail = std::string(argv[2]) + " " + r.value.detail;
            static_cast<void>(host.Audit().Append(e));
        }
        host.Close();
        return 0;
    }


    if (cmd == "project-summary")
    {
        bool save = false;
        std::filesystem::path root = std::filesystem::current_path();
        for (int i = 2; i < argc; ++i)
        {
            if (std::string(argv[i]) == "--save")
                save = true;
            else
                root = argv[i];
        }
        std::vector<std::string> paths;
        const char* exts[] = {".hpp", ".h", ".cpp", ".cc", ".c", ".md", ".py", ".rs", ".go", ".ts", ".js"};
        std::error_code ec;
        if (std::filesystem::is_directory(root, ec))
        {
            for (auto it = std::filesystem::recursive_directory_iterator(
                     root,
                     std::filesystem::directory_options::skip_permission_denied,
                     ec);
                 it != std::filesystem::recursive_directory_iterator();
                 it.increment(ec))
            {
                if (ec)
                {
                    ec.clear();
                    continue;
                }
                if (!it->is_regular_file())
                    continue;
                const auto pth = it->path();
                const auto name = pth.filename().string();
                if (name == "CMakeLists.txt" || name.ends_with(".cmake"))
                {
                    paths.push_back(pth.string());
                }
                else
                {
                    const auto ext = pth.extension().string();
                    for (const char* e : exts)
                    {
                        if (ext == e)
                        {
                            paths.push_back(pth.string());
                            break;
                        }
                    }
                }
                if (paths.size() >= 40)
                    break;
            }
        }
        std::ostringstream user;
        user << "Summarize this project structure and main responsibilities.\n";
        user << "Root: " << root.string() << "\nFiles included: " << paths.size() << "\n";
        constexpr std::size_t kMaxTotal = 48 * 1024;
        std::size_t total = 0;
        for (const auto& path : paths)
        {
            if (total >= kMaxTotal)
                break;
            std::ifstream in(path);
            if (!in)
                continue;
            std::string body;
            char buf[4096];
            while (in && body.size() < 4096)
            {
                in.read(buf, sizeof(buf));
                body.append(buf, static_cast<std::size_t>(in.gcount()));
            }
            user << "\n----- " << path << " -----\n" << body;
            total += body.size();
        }
        sovereign::model_router::ChatRequest req;
        req.model = host.DefaultModel();
        req.messages.push_back(sovereign::model_router::ChatMessage{
            "system",
            "You are Sovereign project analyst. Produce a concise architecture summary. "
            "No system changes.",
            {},
            {}});
        req.messages.push_back(
            sovereign::model_router::ChatMessage{"user", user.str(), {}, {}});
        auto r = host.Models().Chat(req);
        if (!r.Ok() || !r.value.ok)
        {
            std::cerr << "project-summary failed (is Ollama up? model="
                      << host.DefaultModel() << ")\n";
            host.Close();
            return 1;
        }
        std::cout << r.value.content << '\n';
        if (save)
        {
            sovereign::memory::MemoryRecord rec;
            rec.tags = {"project-summary"};
            rec.body = r.value.content;
            auto app = host.Memory().Append(rec);
            if (app.Ok())
                std::cout << "saved_record_id=" << ToHex(app.value.bytes) << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "review-diff" && argc >= 3)
    {
        bool save = false;
        std::string path;
        for (int i = 2; i < argc; ++i)
        {
            if (std::string(argv[i]) == "--save")
                save = true;
            else
                path = argv[i];
        }
        if (path.empty())
        {
            std::cerr << "review-diff requires a file\n";
            host.Close();
            return 2;
        }
        std::ifstream in(path);
        if (!in)
        {
            std::cerr << "cannot read diff file\n";
            host.Close();
            return 1;
        }
        std::string body;
        char buf[4096];
        while (in && body.size() < 64 * 1024)
        {
            in.read(buf, sizeof(buf));
            body.append(buf, static_cast<std::size_t>(in.gcount()));
        }
        sovereign::model_router::ChatRequest req;
        req.model = host.DefaultModel();
        req.messages.push_back(sovereign::model_router::ChatMessage{
            "system",
            "You are a careful code reviewer. Identify bugs, risks, and missing tests. "
            "Be concise. No system changes.",
            {},
            {}});
        req.messages.push_back(sovereign::model_router::ChatMessage{
            "user",
            std::string("Review this unified diff:\n") + body,
            {},
            {}});
        auto r = host.Models().Chat(req);
        if (!r.Ok() || !r.value.ok)
        {
            std::cerr << "review-diff failed (Ollama up? model="
                      << host.DefaultModel() << ")\n";
            host.Close();
            return 1;
        }
        std::cout << r.value.content << '\n';
        if (save)
        {
            sovereign::memory::MemoryRecord rec;
            rec.tags = {"review-diff"};
            rec.body = r.value.content;
            auto app = host.Memory().Append(rec);
            if (app.Ok())
                std::cout << "saved_record_id=" << ToHex(app.value.bytes) << '\n';
        }
        host.Close();
        return 0;
    }


    if (cmd == "brief")
    {
        auto b = host.Briefing().Build();
        std::cout << b.rendered_text;
        host.Close();
        return 0;
    }

    if (cmd == "goal-add" && argc >= 4)
    {
        sovereign::goals::Goal g;
        g.priority = std::atoi(argv[2]);
        std::string title;
        for (int i = 3; i < argc; ++i)
        {
            if (i > 3)
                title.push_back(' ');
            title += argv[i];
        }
        g.title = title;
        std::array<std::uint8_t, 16> session_bytes{};
        if (LoadSession(cfg.data_root, session_bytes))
        {
            sovereign::identity::SessionId sid{};
            sid.bytes = session_bytes;
            auto session = host.Identity().ValidateSession(sid);
            if (session.Ok())
                g.owner = session.value.principal_id;
        }
        auto r = host.Goals().Create(g);
        if (!r.Ok())
        {
            std::cerr << "goal-add failed\n";
            host.Close();
            return 1;
        }
        std::cout << "goal_id=";
        // print hex
        static constexpr char k[] = "0123456789abcdef";
        for (auto b : r.value.bytes)
        {
            std::cout << k[(b >> 4) & 0xF] << k[b & 0xF];
        }
        std::cout << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "goal-list")
    {
        auto r = host.Goals().ListAll();
        if (!r.Ok())
        {
            host.Close();
            return 1;
        }
        for (const auto& g : r.value)
        {
            static constexpr char k[] = "0123456789abcdef";
            for (auto b : g.id.bytes)
                std::cout << k[(b >> 4) & 0xF] << k[b & 0xF];
            std::cout << ' ' << g.priority << ' '
                      << static_cast<int>(g.status) << ' ' << g.title << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "goal-complete" && argc >= 3)
    {
        sovereign::goals::GoalId id{};
        if (!FromHex(argv[2], id.bytes))
        {
            std::cerr << "bad goal id\n";
            host.Close();
            return 1;
        }
        auto r = host.Goals().SetStatus(id, sovereign::goals::GoalStatus::Completed);
        if (!r.Ok())
        {
            std::cerr << "goal-complete failed\n";
            host.Close();
            return 1;
        }
        std::cout << "completed\n";
        host.Close();
        return 0;
    }


    if (cmd == "memory-search" && argc >= 3)
    {
        std::string q;
        for (int i = 2; i < argc; ++i)
        {
            if (i > 2)
                q.push_back(' ');
            q += argv[i];
        }
        auto r = host.Memory().Search(q);
        if (!r.Ok())
        {
            std::cerr << "memory-search failed\n";
            host.Close();
            return 1;
        }
        for (const auto& rec : r.value)
            std::cout << rec.body << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "workspace-init-domains")
    {
        const char* domains[][2] = {
            {"development", "development"},
            {"knowledge", "knowledge"},
            {"business", "business"},
            {"finance", "finance"},
            {"health", "health"},
            {"infrastructure", "infrastructure"},
            {"life", "life"},
        };
        for (auto& d : domains)
        {
            auto r = host.Workspace().Create(d[0], d[1]);
            if (r.Ok())
                std::cout << "created " << d[0] << '\n';
            else
                std::cout << "skip " << d[0] << " (exists or error)\n";
        }
        host.Close();
        return 0;
    }

    if (cmd == "agent-add-brief")
    {
        sovereign::agent::ScheduledJob j;
        j.kind = sovereign::agent::JobKind::BriefSnapshot;
        j.name = "daily-brief";
        j.interval_seconds = argc >= 3 ? std::stoull(argv[2]) : 86400ULL;
        j.enabled = true;
        auto r = host.Agent().Add(j);
        if (!r.Ok())
        {
            std::cerr << "agent-add-brief failed\n";
            host.Close();
            return 1;
        }
        std::cout << "job_added interval=" << j.interval_seconds << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "agent-list")
    {
        auto r = host.Agent().List();
        if (!r.Ok())
        {
            host.Close();
            return 1;
        }
        for (const auto& j : r.value)
        {
            std::cout << j.name << " kind=" << static_cast<int>(j.kind)
                      << " interval=" << j.interval_seconds
                      << " last=" << j.last_run_unix
                      << " enabled=" << (j.enabled ? 1 : 0) << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "agent-tick")
    {
        using namespace std::chrono;
        const auto now = static_cast<std::uint64_t>(
            duration_cast<seconds>(system_clock::now().time_since_epoch())
                .count());
        auto due = host.Agent().Due(now);
        if (!due.Ok())
        {
            host.Close();
            return 1;
        }
        int ran = 0;
        for (const auto& j : due.value)
        {
            if (j.kind == sovereign::agent::JobKind::BriefSnapshot ||
                j.kind == sovereign::agent::JobKind::MemoryDigest)
            {
                auto brief = host.Briefing().Build();
                sovereign::memory::MemoryRecord rec;
                rec.tags = {"agent", "brief", j.name};
                rec.body = brief.rendered_text;
                auto app = host.Memory().Append(rec);
                if (app.Ok())
                {
                    std::cout << "ran " << j.name << " saved_memory\n";
                    ++ran;
                }
            }
            sovereign::agent::JobId id = j.id;
            static_cast<void>(host.Agent().MarkRun(id, now));
        }
        std::cout << "agent-tick jobs_run=" << ran << '\n';
        host.Close();
        return 0;
    }


    if (cmd == "semantic-add" && argc >= 3)
    {
        std::string text;
        for (int i = 2; i < argc; ++i)
        {
            if (i > 2)
                text.push_back(' ');
            text += argv[i];
        }
        auto r = host.Semantic().Upsert(text, {"cli"});
        if (!r.Ok())
        {
            std::cerr << "semantic-add failed\n";
            host.Close();
            return 1;
        }
        std::cout << "semantic_ok\n"; // model recorded in store
        host.Close();
        return 0;
    }

    if (cmd == "semantic-query" && argc >= 3)
    {
        std::string q;
        for (int i = 2; i < argc; ++i)
        {
            if (i > 2)
                q.push_back(' ');
            q += argv[i];
        }
        auto r = host.Semantic().Query(q, 5);
        if (!r.Ok())
        {
            std::cerr << "semantic-query failed\n";
            host.Close();
            return 1;
        }
        for (const auto& s : r.value)
            std::cout << s.score << ' ' << s.record.text << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "domain-show")
    {
        if (argc >= 3)
        {
            auto* prof = sovereign::domain::FindProfile(argv[2]);
            if (!prof)
            {
                std::cerr << "unknown domain\n";
                host.Close();
                return 1;
            }
            std::cout << "name=" << prof->name << '\n';
            std::cout << "prompt=" << prof->system_prompt << '\n';
            std::cout << "tools=";
            for (std::size_t i = 0; i < prof->allowed_tools.size(); ++i)
            {
                if (i)
                    std::cout << ',';
                std::cout << prof->allowed_tools[i];
            }
            std::cout << '\n';
        }
        else
        {
            for (const auto& prof : sovereign::domain::BuiltinProfiles())
                std::cout << prof.name << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "domain-chat" && argc >= 4)
    {
        auto* profile = sovereign::domain::FindProfile(argv[2]);
        if (!profile)
        {
            std::cerr << "unknown domain\n";
            host.Close();
            return 1;
        }
        std::string prompt;
        for (int i = 3; i < argc; ++i)
        {
            if (i > 3)
                prompt.push_back(' ');
            prompt += argv[i];
        }
        std::string context;
        auto sem = host.Semantic().Query(prompt, 3);
        if (sem.Ok())
        {
            for (const auto& s : sem.value)
            {
                if (s.score > 0.05f)
                    context += s.record.text + "\n";
            }
        }
        sovereign::model_router::ChatRequest req;
        req.model = host.DefaultModel();
        req.messages.push_back(sovereign::model_router::ChatMessage{
            "system", profile->system_prompt, {}, {}});
        if (!context.empty())
            req.messages.push_back(sovereign::model_router::ChatMessage{
                "system", std::string("Relevant memory:\n") + context, {}, {}});
        req.messages.push_back(
            sovereign::model_router::ChatMessage{"user", prompt, {}, {}});
        for (const auto& tool : profile->allowed_tools)
        {
            sovereign::model_router::ToolDefinition td;
            td.name = tool;
            td.description = "Domain-allowed tool: " + tool;
            td.parameters_json = "{\"type\":\"object\",\"properties\":{}}";
            req.tools.push_back(td);
        }
        auto r = host.Models().Chat(req);
        if (!r.Ok() || !r.value.ok)
        {
            std::cerr << "domain-chat failed (Ollama up?)\n";
            host.Close();
            return 1;
        }
        std::cout << r.value.content << '\n';
        sovereign::domain::DomainToolRunner runner(
            host.Memory(), host.Semantic());
        for (const auto& tc : r.value.tool_calls)
        {
            sovereign::domain::ToolCallRequest req;
            req.name = tc.name;
            req.arguments_json = tc.arguments_json;
            auto tr = runner.Run(*profile, req);
            if (!tr.allowed)
                std::cout << "tool_blocked name=" << tc.name << '\n';
            else
                std::cout << "tool_result name=" << tc.name
                          << " ok=" << (tr.ok ? 1 : 0)
                          << " detail=" << tr.detail << '\n';
        }
        host.Close();
        return 0;
    }


    if (cmd == "peer-export" && argc >= 3)
    {
        sovereign::PrincipalId actor{};
        actor.bytes[0] = 1;
        auto r = host.Peer().ExportTo(argv[2], actor);
        if (!r.Ok())
        {
            std::cerr << "peer-export failed\n";
            host.Close();
            return 1;
        }
        std::cout << "bundle=" << r.value.path << '\n';
        std::cout << "goals=" << r.value.goals << " memory=" << r.value.memory
                  << " audit=" << r.value.audit << '\n';
        host.Close();
        return 0;
    }

    if (cmd == "peer-import" && argc >= 3)
    {
        sovereign::PrincipalId actor{};
        actor.bytes[0] = 1;
        auto r = host.Peer().ImportFrom(argv[2], actor);
        if (!r.Ok())
        {
            std::cerr << "peer-import failed\n";
            host.Close();
            return 1;
        }
        std::cout << "imported goals=" << r.value.goals
                  << " memory=" << r.value.memory
                  << " semantic=" << r.value.semantic << '\n';
        host.Close();
        return 0;
    }


    if (cmd == "inspect")
    {
        const std::string section = argc >= 3 ? argv[2] : "all";
        sovereign::diagnostics::TransparencyService tr(host);
        auto report = tr.BuildSection(section);
        std::cout << report.text;
        host.Close();
        return 0;
    }

    if (cmd == "ask" && argc >= 3)
    {
        std::string q;
        for (int i = 2; i < argc; ++i)
        {
            if (i > 2)
                q.push_back(' ');
            q += argv[i];
        }
        // Map natural questions to transparency sections
        std::string section = "all";
        auto has = [&](const char* s) {
            return q.find(s) != std::string::npos;
        };
        if (has("identity") || has("authority") || has("who"))
            section = "identity";
        else if (has("memory") || has("provenance") || has("remember"))
            section = "memory";
        else if (has("model") || has("provider") || has("routing") || has("ollama"))
            section = "model";
        else if (has("policy"))
            section = "policy";
        else if (has("plan") || has("pending") || has("goal"))
            section = "plans";
        else if (has("approv") || has("denial") || has("denied"))
            section = "approvals";
        else if (has("execut") || has("command"))
            section = "executions";
        else if (has("state") || has("workspace"))
            section = "state";
        else if (has("timing") || has("resource") || has("uptime"))
            section = "timing";
        else if (has("error") || has("recovery"))
            section = "errors";
        else if (has("audit") || has("evidence"))
            section = "audit";
        else if (has("health") || has("subsystem"))
            section = "health";
        else if (has("coverage") || has("spec") || has("complete"))
            section = "coverage";
        sovereign::diagnostics::TransparencyService tr(host);
        std::cout << "question=" << q << "\nsection=" << section << "\n";
        std::cout << tr.BuildSection(section).text;
        host.Close();
        return 0;
    }

    if (cmd == "desktop-status")
    {
        const bool as_json = (argc >= 3 && std::string(argv[2]) == "--json");
        const auto st = host.ApiGateway().GetDesktopStatus(10);
        if (as_json)
        {
            std::cout << "{\"mode\":\"" << st.mode << "\","
                      << "\"has_display\":" << (st.has_display ? "true" : "false") << ","
                      << "\"computer_use_enabled\":" << (st.computer_use_enabled ? "true" : "false") << ","
                      << "\"active_purpose_id\":\"" << st.active_purpose_id << "\","
                      << "\"pending_decisions\":" << st.pending_decisions << ","
                      << "\"evidence_files\":" << st.evidence_files << ","
                      << "\"recent_audit_count\":" << st.recent_audit_events.size() << "}\n";
        }
        else
        {
            std::cout << "mode=" << st.mode << "\n"
                      << "has_display=" << (st.has_display ? "1" : "0") << "\n"
                      << "computer_use_enabled=" << (st.computer_use_enabled ? "1" : "0") << "\n"
                      << "active_purpose_id=" << st.active_purpose_id << "\n"
                      << "pending_decisions=" << st.pending_decisions << "\n"
                      << "evidence_files=" << st.evidence_files << "\n"
                      << "recent_audit_count=" << st.recent_audit_events.size() << "\n";
            for (const auto& ev : st.recent_audit_events)
            {
                std::cout << "audit=" << ev << "\n";
            }
        }
        host.Close();
        return 0;
    }

    if (cmd == "session-ensure")
    {
        const bool as_json = (argc >= 3 && std::string(argv[2]) == "--json");
        auto probe = [](const sovereign::os_runtime::SessionServiceEndpoint& ep) -> bool {
            if (ep.port == 0)
                return std::system("pgrep -f sovereign-daemon >/dev/null 2>&1") == 0;
            const std::string cmd_str = "curl -4 -fsS --noproxy \"*\" --max-time 1 " + ep.health_url + " >/dev/null 2>&1";
            return std::system(cmd_str.c_str()) == 0;
        };
        const auto rep = sovereign::os_runtime::ServiceOrchestrator::EvaluateSessionReadiness(probe);
        if (as_json)
        {
            std::cout << "{\"all_ready\":" << (rep.all_ready ? "true" : "false")
                      << ",\"ready_count\":" << rep.ready_count
                      << ",\"total\":" << rep.services.size() << ",\"services\":[";
            for (std::size_t i = 0; i < rep.services.size(); ++i)
            {
                const auto& s = rep.services[i];
                if (i > 0) std::cout << ",";
                std::cout << "{\"name\":\"" << s.name << "\",\"port\":" << s.port
                          << ",\"ready\":" << (s.ready ? "true" : "false") << "}";
            }
            std::cout << "]}\n";
        }
        else
        {
            std::cout << "all_ready=" << (rep.all_ready ? "1" : "0")
                      << " ready_count=" << rep.ready_count << "/" << rep.services.size() << "\n";
            for (const auto& s : rep.services)
                std::cout << "service=" << s.name << " port=" << s.port << " ready=" << (s.ready ? "1" : "0") << "\n";
        }
        host.Close();
        return 0;
    }

    if (cmd == "plan-route" && argc >= 4)
    {
        bool as_json = false;
        int idx = 2;
        if (std::string(argv[idx]) == "--json" && argc >= 5) { as_json = true; ++idx; }
        const std::string ws = argv[idx];
        const std::string act = argv[idx + 1];
        const std::string cap = sovereign::capability::CapabilityGate::CapabilityForAction(act);
        sovereign::node::NodeRegistry reg;
        sovereign::node::PlanRouteDecision dec;
        if (reg.Open(host.DataRoot()) == sovereign::node::NodeError::Ok)
        {
            const auto r = reg.RouteWorkspaceAction(ws, act, cap);
            if (r.Ok()) dec = r.value;
            reg.Close();
        }
        if (as_json)
        {
            std::cout << "{\"allowed\":" << (dec.allowed ? "true" : "false")
                      << ",\"workspace\":\"" << dec.workspace << "\",\"action\":\"" << dec.action
                      << "\",\"capability_id\":\"" << dec.capability_id << "\",\"node\":\"" << dec.node_name
                      << "\",\"endpoint\":\"" << dec.endpoint << "\",\"reason\":\"" << dec.reason << "\"}\n";
        }
        else
        {
            std::cout << "allowed=" << (dec.allowed ? "1" : "0") << " workspace=" << dec.workspace
                      << " action=" << dec.action << " capability_id=" << dec.capability_id
                      << " node=" << dec.node_name << " endpoint=" << dec.endpoint
                      << " reason=" << dec.reason << "\n";
        }
        host.Close();
        return dec.allowed ? 0 : 1;
    }

    if (cmd == "node-execute" && argc >= 4)
    {
        bool as_json = false;
        int idx = 2;
        if (std::string(argv[idx]) == "--json" && argc >= 5) { as_json = true; ++idx; }
        const std::string ws = argv[idx];
        const std::string act = argv[idx + 1];
        std::string payload;
        for (int i = idx + 2; i < argc; ++i) { if (!payload.empty()) payload.push_back(' '); payload += argv[i]; }
        const auto gate_dec = host.Gate().RequireForAction(act);
        sovereign::node::NodeExecutionReport rep;
        rep.workspace = ws;
        rep.action = act;
        rep.capability_id = gate_dec.capability_id;
        if (!gate_dec.allowed)
        {
            rep.reason = gate_dec.reason.empty() ? "capability_denied" : gate_dec.reason;
        }
        else
        {
            const auto res = host.Nodes().ExecuteOnBoundNode(ws, act, gate_dec.capability_id, payload);
            if (res.Ok()) rep = res.value;
        }
        sovereign::audit::AuditEntry ae;
        ae.category = "federation";
        ae.action = rep.executed ? "node.execute.allowed" : "node.execute.denied";
        ae.subject = rep.node_id_hex.empty() ? ws : rep.node_id_hex;
        ae.detail = "ws=" + ws + " action=" + act + " reason=" + rep.reason;
        (void)host.Audit().Append(ae);
        if (as_json)
        {
            std::cout << "{\"executed\":" << (rep.executed ? "true" : "false")
                      << ",\"workspace\":\"" << rep.workspace << "\",\"action\":\"" << rep.action
                      << "\",\"node_id\":\"" << rep.node_id_hex << "\",\"node\":\"" << rep.node_name
                      << "\",\"reason\":\"" << rep.reason << "\",\"audit_logged\":true}\n";
        }
        else
        {
            std::cout << "executed=" << (rep.executed ? "1" : "0") << " workspace=" << rep.workspace
                      << " action=" << rep.action << " node_id=" << rep.node_id_hex
                      << " node=" << rep.node_name << " reason=" << rep.reason << " audit_logged=1\n";
        }
        host.Close();
        return rep.executed ? 0 : 1;
    }

    if ((cmd == "core-backup" || cmd == "core-restore") && argc >= 3)
    {
        bool as_json = false;
        int idx = 2;
        if (std::string(argv[idx]) == "--json" && argc >= 4) { as_json = true; ++idx; }
        const std::filesystem::path snap_dir = argv[idx];
        const auto rep = (cmd == "core-backup")
            ? sovereign::archive::ArchiveService::CreateCoreSnapshot(host.DataRoot(), snap_dir)
            : sovereign::archive::ArchiveService::RestoreCoreSnapshot(snap_dir, host.DataRoot());
        sovereign::audit::AuditEntry ae;
        ae.category = "ha";
        ae.action = cmd;
        ae.subject = snap_dir.string();
        ae.detail = "ok=" + std::string(rep.ok ? "1" : "0") + " checksum=" + rep.checksum_hex + " detail=" + rep.detail;
        (void)host.Audit().Append(ae);
        if (as_json)
        {
            std::cout << "{\"ok\":" << (rep.ok ? "true" : "false")
                      << ",\"op\":\"" << cmd << "\",\"files\":" << rep.file_count
                      << ",\"bytes\":" << rep.total_bytes << ",\"checksum\":\"" << rep.checksum_hex
                      << "\",\"detail\":\"" << rep.detail << "\"}\n";
        }
        else
        {
            std::cout << "ok=" << (rep.ok ? "1" : "0") << " op=" << cmd << " files=" << rep.file_count
                      << " bytes=" << rep.total_bytes << " checksum=" << rep.checksum_hex
                      << " detail=" << rep.detail << "\n";
        }
        host.Close();
        return rep.ok ? 0 : 1;
    }

    if (cmd == "profile-check")
    {
        const bool as_json = (argc >= 3 && std::string(argv[2]) == "--json");
        const char* env_prof = std::getenv("SOVEREIGN_PROFILE");
        const bool is_nixos = std::filesystem::exists("/etc/NIXOS") || (env_prof != nullptr && std::string(env_prof) == "nixos-sovereign");
        const std::string profile = is_nixos ? "nixos-sovereign" : "host-pop";
        const bool has_flake = std::filesystem::exists("flake.nix") || std::filesystem::exists("../flake.nix");
        const bool has_node_mod = std::filesystem::exists("nix/modules/sovereign-node.nix") || std::filesystem::exists("../nix/modules/sovereign-node.nix");
        if (as_json)
        {
            std::cout << "{\"profile\":\"" << profile << "\",\"is_nixos\":" << (is_nixos ? "true" : "false")
                      << ",\"flake_present\":" << (has_flake ? "true" : "false")
                      << ",\"node_module_present\":" << (has_node_mod ? "true" : "false") << "}\n";
        }
        else
        {
            std::cout << "profile=" << profile << " is_nixos=" << (is_nixos ? "1" : "0")
                      << " flake_present=" << (has_flake ? "1" : "0")
                      << " node_module_present=" << (has_node_mod ? "1" : "0") << "\n";
        }
        host.Close();
        return 0;
    }

    if (cmd == "fleet-status")
    {
        const bool as_json = (argc >= 3 && std::string(argv[2]) == "--json");
        const auto fs = host.Nodes().GetFleetSummary();
        if (as_json)
        {
            std::cout << "{\"total_nodes\":" << fs.total_nodes << ",\"healthy_nodes\":" << fs.healthy_nodes
                      << ",\"bound_workspaces\":" << fs.bound_workspaces << ",\"target_capacity\":" << fs.target_capacity << "}\n";
        }
        else
        {
            std::cout << "total_nodes=" << fs.total_nodes << " healthy_nodes=" << fs.healthy_nodes
                      << " bound_workspaces=" << fs.bound_workspaces << " target_capacity=" << fs.target_capacity << "\n";
        }
        host.Close();
        return 0;
    }
    if (cmd == "node-replace" && argc >= 4)
    {
        bool as_json = false;
        int idx = 2;
        if (std::string(argv[idx]) == "--json" && argc >= 5) { as_json = true; ++idx; }
        const std::string ws = argv[idx];
        const std::string new_name = argv[idx + 1];
        const std::string ep = (argc > idx + 2) ? argv[idx + 2] : "";
        const auto r = host.Nodes().ReplaceWorkspaceNode(ws, new_name, ep);
        const std::string hex = r.Ok() ? sovereign::node::NodeRegistry::IdHex(r.value) : "";
        sovereign::audit::AuditEntry ae;
        ae.category = "federation";
        ae.action = "node.replace";
        ae.subject = hex.empty() ? ws : hex;
        ae.detail = "ws=" + ws + " new_node=" + new_name + " ok=" + std::string(r.Ok() ? "1" : "0");
        (void)host.Audit().Append(ae);
        if (as_json)
            std::cout << "{\"ok\":" << (r.Ok() ? "true" : "false") << ",\"workspace\":\"" << ws << "\",\"node\":\"" << new_name << "\",\"node_id\":\"" << hex << "\"}\n";
        else
            std::cout << "ok=" << (r.Ok() ? "1" : "0") << " workspace=" << ws << " node=" << new_name << " node_id=" << hex << "\n";
        host.Close();
        return r.Ok() ? 0 : 1;
    }

    if (cmd == "verify-outcome" && argc >= 5)
    {
        bool as_json = false;
        int idx = 2;
        if (std::string(argv[idx]) == "--json" && argc >= 6) { as_json = true; ++idx; }
        const std::string act = argv[idx];
        const std::string exp = argv[idx + 1];
        std::string obs;
        for (int i = idx + 2; i < argc; ++i) { if (!obs.empty()) obs.push_back(' '); obs += argv[i]; }
        const auto rec = sovereign::executor::ActionExecutor::VerifyOutcome(act, "token_match", exp, obs);
        sovereign::audit::AuditEntry ae;
        ae.category = "verification";
        ae.action = rec.verified ? "outcome.verified" : "outcome.failed";
        ae.subject = act;
        ae.detail = "hash=" + rec.evidence_hash + " detail=" + rec.observed_detail;
        (void)host.Audit().Append(ae);
        if (as_json)
            std::cout << "{\"verified\":" << (rec.verified ? "true" : "false") << ",\"action\":\"" << rec.action
                      << "\",\"evidence_hash\":\"" << rec.evidence_hash << "\",\"detail\":\"" << rec.observed_detail << "\"}\n";
        else
            std::cout << "verified=" << (rec.verified ? "1" : "0") << " action=" << rec.action
                      << " evidence_hash=" << rec.evidence_hash << " detail=" << rec.observed_detail << "\n";
        host.Close();
        return rec.verified ? 0 : 1;
    }
    if (cmd == "goal-execute" && argc >= 4)
    {
        bool as_json = false;
        int idx = 2;
        if (std::string(argv[idx]) == "--json" && argc >= 5) { as_json = true; ++idx; }
        const std::string ws = argv[idx];
        std::string goal;
        for (int i = idx + 1; i < argc; ++i) { if (!goal.empty()) goal.push_back(' '); goal += argv[i]; }
        const std::string act = "browser_open";
        const auto gate = host.Gate().RequireForAction(act);
        sovereign::node::NodeExecutionReport ex;
        if (gate.allowed)
        {
            const auto r = host.Nodes().ExecuteOnBoundNode(ws, act, gate.capability_id, goal);
            if (r.Ok()) ex = r.value;
        }
        const auto ver = sovereign::executor::ActionExecutor::VerifyOutcome(
            act, "non_empty_output", ex.executed ? "executed_on_bound_node" : "unroutable", ex.executed ? ex.reason : "");
        sovereign::audit::AuditEntry ae;
        ae.category = "closed_loop";
        ae.action = ver.verified ? "goal.verified" : "goal.unverified";
        ae.subject = ws;
        ae.detail = "goal=" + goal + " node=" + ex.node_name + " hash=" + ver.evidence_hash;
        (void)host.Audit().Append(ae);
        sovereign::memory::MemoryRecord mr;
        mr.tags = {"closed_loop", ver.verified ? "verified" : "failed", ws};
        mr.body = "goal=" + goal + " node=" + ex.node_name + " verified=" + std::string(ver.verified ? "1" : "0") + " hash=" + ver.evidence_hash;
        (void)host.Memory().Append(mr);
        if (as_json)
            std::cout << "{\"verified\":" << (ver.verified ? "true" : "false") << ",\"workspace\":\"" << ws
                      << "\",\"node\":\"" << ex.node_name << "\",\"evidence_hash\":\"" << ver.evidence_hash
                      << "\",\"memory_recorded\":true}\n";
        else
            std::cout << "verified=" << (ver.verified ? "1" : "0") << " workspace=" << ws
                      << " node=" << ex.node_name << " evidence_hash=" << ver.evidence_hash << " memory_recorded=1\n";
        host.Close();
        return ver.verified ? 0 : 1;
    }

    if (cmd == "curiosity-probe" && argc >= 4)
    {
        bool as_json = false;
        int idx = 2;
        if (std::string(argv[idx]) == "--json" && argc >= 5) { as_json = true; ++idx; }
        const std::string probe_act = argv[idx];
        std::string topic;
        for (int i = idx + 1; i < argc; ++i) { if (!topic.empty()) topic.push_back(' '); topic += argv[i]; }
        sovereign::PrincipalId actor{};
        const auto rep = host.Curiosity().RunBoundedProbe(actor, probe_act, topic);
        sovereign::audit::AuditEntry ae;
        ae.category = "curiosity";
        ae.action = rep.allowed ? "probe.allowed" : "probe.denied";
        ae.subject = rep.topic;
        ae.detail = "action=" + probe_act + " reason=" + rep.reason;
        (void)host.Audit().Append(ae);
        if (rep.allowed)
        {
            sovereign::memory::MemoryRecord mr;
            mr.tags = {"curiosity", "bounded_probe"};
            mr.body = "topic=" + rep.topic + " action=" + probe_act + " questions=" + std::to_string(rep.questions.size());
            (void)host.Memory().Append(mr);
        }
        if (as_json)
        {
            std::cout << "{\"allowed\":" << (rep.allowed ? "true" : "false")
                      << ",\"probe_action\":\"" << rep.probe_action << "\",\"topic\":\"" << rep.topic
                      << "\",\"questions\":" << rep.questions.size() << ",\"reason\":\"" << rep.reason << "\"}\n";
        }
        else
        {
            std::cout << "allowed=" << (rep.allowed ? "1" : "0") << " probe_action=" << rep.probe_action
                      << " topic=" << rep.topic << " questions=" << rep.questions.size()
                      << " reason=" << rep.reason << "\n";
        }
        host.Close();
        return rep.allowed ? 0 : 1;
    }

    if (cmd == "inventory-sync")
    {
        std::vector<std::string> apps;
        if (argc >= 4 && std::string(argv[2]) == "--apps")
        {
            std::string s = argv[3], cur;
            for (char c : s) { if (c == ',') { if (!cur.empty()) apps.push_back(cur); cur.clear(); } else cur.push_back(c); }
            if (!cur.empty()) apps.push_back(cur);
        }
        else
        {
            for (const auto& dir : {std::filesystem::path("/usr/share/applications"), std::filesystem::path(std::getenv("HOME") ? std::getenv("HOME") : "") / ".local/share/applications"})
            {
                std::error_code ec;
                if (!std::filesystem::exists(dir, ec)) continue;
                for (const auto& e : std::filesystem::directory_iterator(dir, ec))
                    if (e.path().extension() == ".desktop") apps.push_back(e.path().stem().string());
            }
        }
        const auto r = host.Policy().SyncInventoryAllowlist(apps);
        sovereign::audit::AuditEntry ae;
        ae.category = "inventory";
        ae.action = "inventory.sync";
        ae.subject = "allowlist";
        ae.detail = "count=" + std::to_string(r.value);
        (void)host.Audit().Append(ae);
        std::cout << "synced_apps=" << r.value << "\n";
        host.Close();
        return r.Ok() ? 0 : 1;
    }
    if (cmd == "app-check" && argc >= 3)
    {
        bool as_json = false;
        int idx = 2;
        if (std::string(argv[idx]) == "--json" && argc >= 4) { as_json = true; ++idx; }
        const std::string app_id = argv[idx];
        const bool ok = host.Policy().IsInventoryAppAllowed(app_id);
        sovereign::audit::AuditEntry ae;
        ae.category = "inventory";
        ae.action = ok ? "app_open.allowed" : "app_open.denied";
        ae.subject = app_id;
        ae.detail = ok ? "in_unified_inventory" : "not_in_unified_inventory";
        (void)host.Audit().Append(ae);
        if (as_json)
            std::cout << "{\"allowed\":" << (ok ? "true" : "false") << ",\"app_id\":\"" << app_id
                      << "\",\"reason\":\"" << (ok ? "in_unified_inventory" : "not_in_unified_inventory") << "\"}\n";
        else
            std::cout << "allowed=" << (ok ? "1" : "0") << " app_id=" << app_id
                      << " reason=" << (ok ? "in_unified_inventory" : "not_in_unified_inventory") << "\n";
        host.Close();
        return ok ? 0 : 1;
    }

    if (cmd == "purpose-seed")
    {
        const auto added = host.Purpose().SeedDefaultLibrary(host.DataRoot() / "purposes");
        sovereign::audit::AuditEntry ae;
        ae.category = "purpose";
        ae.action = "purpose.library.seed";
        ae.subject = "library";
        ae.detail = "added=" + std::to_string(added);
        (void)host.Audit().Append(ae);
        std::cout << "seeded_purposes=" << added << " total_purposes=" << host.Purpose().BuiltinPacks().size() << "\n";
        host.Close();
        return 0;
    }
    if (cmd == "purpose-inspect" && argc >= 3)
    {
        bool as_json = false;
        int idx = 2;
        if (std::string(argv[idx]) == "--json" && argc >= 4) { as_json = true; ++idx; }
        const std::string pid = argv[idx];
        (void)host.Purpose().SeedDefaultLibrary({});
        const auto found = host.Purpose().Find(pid);
        if (!found.has_value())
        {
            std::cerr << "purpose not found: " << pid << "\n";
            host.Close();
            return 1;
        }
        const auto inv = host.Purpose().ValidateAgainstInventory(pid, host.Policy().ListInventoryAllowlist());
        if (as_json)
            std::cout << "{\"purpose_id\":\"" << found->purpose_id << "\",\"require_headed\":" << (found->require_headed ? "true" : "false")
                      << ",\"inventory_compliant\":" << (inv.allowed ? "true" : "false") << ",\"reason\":\"" << inv.reason << "\"}\n";
        else
            std::cout << "purpose_id=" << found->purpose_id << " require_headed=" << (found->require_headed ? "1" : "0")
                      << " inventory_compliant=" << (inv.allowed ? "1" : "0") << " reason=" << inv.reason << "\n";
        host.Close();
        return 0;
    }

    if (cmd == "desktop-drive" && argc >= 4)
    {
        bool as_json = false;
        bool req_headed = true;
        int idx = 2;
        while (idx < argc && std::string(argv[idx]).rfind("--", 0) == 0)
        {
            const std::string fl = argv[idx++];
            if (fl == "--json") as_json = true;
            else if (fl == "--allow-headless") req_headed = false;
        }
        if (argc < idx + 2) { PrintUsage(); host.Close(); return 2; }
        const std::string op = argv[idx];
        const std::string app = argv[idx + 1];
        std::string payload;
        for (int i = idx + 2; i < argc; ++i) { if (!payload.empty()) payload.push_back(' '); payload += argv[i]; }
        sovereign::product::DesktopOperatorGateway gw(host);
        const auto rec = gw.DriveWindow(op, app, payload, req_headed);
        if (as_json)
            std::cout << "{\"executed\":" << (rec.executed ? "true" : "false") << ",\"op\":\"" << rec.op
                      << "\",\"app\":\"" << rec.target_app << "\",\"headed\":" << (rec.headed ? "true" : "false")
                      << ",\"evidence_hash\":\"" << rec.evidence_hash << "\",\"reason\":\"" << rec.reason << "\"}\n";
        else
            std::cout << "executed=" << (rec.executed ? "1" : "0") << " op=" << rec.op
                      << " app=" << rec.target_app << " headed=" << (rec.headed ? "1" : "0")
                      << " evidence_hash=" << rec.evidence_hash << " reason=" << rec.reason << "\n";
        host.Close();
        return rec.executed ? 0 : 1;
    }

    PrintUsage();
    host.Close();
    return 2;
}
