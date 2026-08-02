// Sovereign local CLI — in-process SovereignHost surface (not a network gateway).

#include "sovereign_host.hpp"
#include "transparency_report.hpp"
#include "domain_tool_runner.hpp"
#include "identity_verifier.hpp"

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
        << "  sovereign-cli login <name> <secret>\n"
        << "  sovereign-cli workspace-create <name> <kind>\n"
        << "  sovereign-cli workspace-list\n"
        << "  sovereign-cli memory-add <tag> <body...>\n"
        << "  sovereign-cli memory-list [tag]\n"
        << "  sovereign-cli recommend <action> <summary>\n"
        << "  sovereign-cli loop [--auto-approve] <goal...>\n"
        << "  sovereign-cli approve <decision-hex> <reason>\n"
        << "  sovereign-cli reject <decision-hex> <reason>\n"
        << "  sovereign-cli override <decision-hex> <reason>\n"
        << "  sovereign-cli rule-set <action> <auto:0|1> <min-approve-role>\n"
        << "  sovereign-cli logout\n"
        << "  sovereign-cli operator-create <name> <secret>\n"
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
        << "  sovereign-cli memory-rank\n"
        << "  sovereign-cli memory-prune <min-score>\n"
        << "  sovereign-cli plan <goal...>\n"
        << "  sovereign-cli policy-dry-run <role> <action>\n"
        << "  sovereign-cli policy-version\n"
        << "  sovereign-cli policy-rule-set <action> <min-role> <allow|deny>\n"
        << "  sovereign-cli audit-verify\n"
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

    if (cmd == "status")
    {
        std::cout << "data_root=" << host.DataRoot() << '\n';
        auto models = host.Models().Health();
        std::cout << "ollama="
                  << (models.Ok() && models.value ? "up" : "down") << '\n';
        auto ws = host.Workspace().List();
        std::cout << "workspaces="
                  << (ws.Ok() ? ws.value.size() : 0) << '\n';
        std::cout << "argon2=" << sovereign::identity::DescribeArgon2Params()
                  << '\n';
        std::cout << "surface=cli-only\n";
        std::cout << "sync=peer-bundle\n";
        std::cout << "hermes=separate\n";
        std::cout << "model=" << host.DefaultModel() << '\n';
        {
            const char* em = std::getenv("SOVEREIGN_EMBED_MODEL");
            std::cout << "embed_model="
                      << (em && em[0] ? em : "nomic-embed-text (fallback hash-v1)")
                      << '\n';
        }
        host.Close();
        return 0;
    }

    if (cmd == "bootstrap" && argc >= 4)
    {
        auto r = host.Identity().BootstrapOwner(argv[2], argv[3]);
        if (!r.Ok())
        {
            std::cerr << "bootstrap failed error=" << static_cast<int>(r.error)
                      << '\n';
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
        auto r = host.Identity().Authenticate(argv[2], argv[3]);
        if (!r.Ok())
        {
            std::cerr << "login failed error=" << static_cast<int>(r.error)
                      << '\n';
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
        host.Close();
        return 0;
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
        auto r = host.Cognitive().BuildPlan(actor, goal);
        if (!r.Ok())
        {
            std::cerr << "plan failed\n";
            host.Close();
            return 1;
        }
        std::cout << "goal=" << r.value.goal << '\n';
        std::cout << "confidence=" << r.value.confidence << '\n';
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
            std::cerr << "operator-create failed error="
                      << static_cast<int>(r.error) << '\n';
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
            std::cerr << "service-create failed error="
                      << static_cast<int>(r.error) << '\n';
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
            std::cerr << "machine-create failed error="
                      << static_cast<int>(r.error) << '\n';
            host.Close();
            return 1;
        }
        std::cout << "machine_id=" << ToHex(r.value.bytes) << '\n';
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
        auto r = host.Identity().GetRoles(id);
        if (!r.Ok())
        {
            std::cerr << "roles failed\n";
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
            std::cerr << "change-credential failed error="
                      << static_cast<int>(r.error) << '\n';
            host.Close();
            return 1;
        }
        std::cout << "credential_changed\n";
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
            std::cerr << "principals failed error="
                      << static_cast<int>(r.error) << '\n';
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
        std::array<std::uint8_t, 16> decision{};
        if (!FromHex(argv[3], decision))
        {
            std::cerr << "bad decision id\n";
            host.Close();
            return 1;
        }
        // Register a safe local handler for this process
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

    PrintUsage();
    host.Close();
    return 2;
}
