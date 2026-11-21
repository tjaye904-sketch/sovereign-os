#include "node_registry.hpp"
#include "capability_registry.hpp"
#include "memory_facets.hpp"
#include "secret_manager.hpp"
// Sovereign local CLI — in-process SovereignHost surface (not a network gateway).

#include "sovereign_host.hpp"
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
        << "  sovereign-cli chat-tools <prompt...>\n"
        << "  sovereign-cli execute <action> <decision-hex>\n"
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
    if (!host.Open())
    {
        std::cerr << "failed to open host at " << cfg.data_root << '\n';
        return 1;
    }

    const std::string cmd = argv[1];
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
        << "  sovereign-cli chat-tools <prompt...>\n"
        << "  sovereign-cli execute <action> <decision-hex>\n"
        << "Env: SOVEREIGN_DATA_ROOT, OLLAMA_HOST, SOVEREIGN_OLLAMA_MODEL\n";
