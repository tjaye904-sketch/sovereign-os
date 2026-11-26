#include "workspace_blueprints.hpp"

namespace sovereign::workspace {

const std::vector<WorkspaceBlueprint>& StagedBlueprints() {
    static const std::vector<WorkspaceBlueprint> kBlueprints = {
        {"personal", "Personal Core", "personal_assistant", {"host.sys", "host.fs", "host.voice"}, {"bash", "sqlite3", "tar"}, {"sovereign-mission-control", "konsole"}, {"chat-tools", "voice-route", "core-backup"}},
        {"health", "Health & Biometrics", "wellness_monitor", {"host.sys", "host.fs"}, {"python3", "jq"}, {"firefox", "sovereign-mission-control"}, {"memory-facet-add", "agent-add-brief"}},
        {"education", "Education & Study", "tutor_node", {"host.sys", "host.model"}, {"python3", "pandoc"}, {"firefox", "okular"}, {"curiosity-probe", "semantic-query"}},
        {"finance", "Finance & Ledger", "ledger_node", {"host.sys", "host.fs", "host.audit"}, {"python3", "gnupg", "sha256sum"}, {"libreoffice", "sovereign-mission-control"}, {"audit-verify", "purpose-run"}},
        {"home", "Home & Facility", "facility_hub", {"host.sys", "host.net", "host.voice"}, {"avahi-browse", "ip", "pw-play"}, {"sovereign-voice-talk", "firefox"}, {"voice-route", "fleet-discover"}},
        {"media", "Media & AV", "media_station", {"host.sys", "host.voice", "host.fs"}, {"ffmpeg", "pw-play", "pw-record"}, {"vlc", "audacity"}, {"voice-route", "desktop-drive"}},
        {"communications", "Communications", "comms_relay", {"host.sys", "host.net", "host.voice"}, {"curl", "ssh", "openssl"}, {"firefox", "thunderbird"}, {"voice-route", "peer-sync"}},
        {"development", "Software Engineering", "build_worker", {"host.sys", "host.fs", "host.exec", "host.model"}, {"git", "cmake", "ninja", "g++", "nix"}, {"code", "konsole"}, {"sandbox-propose", "evolve", "fleet-dispatch"}},
        {"operations", "Operations & Fleet", "ops_controller", {"host.sys", "host.net", "host.audit"}, {"systemctl", "journalctl", "ss", "ssh"}, {"sovereign-mission-control", "konsole"}, {"doctor", "fleet-discover", "fleet-dispatch"}},
        {"research", "Research & Discovery", "research_node", {"host.sys", "host.model", "host.net"}, {"python3", "curl", "jq"}, {"firefox", "zotero"}, {"curiosity-probe", "semantic-query", "plan-route"}},
        {"creative", "Creative Studio", "creative_node", {"host.sys", "host.model", "host.fs"}, {"ffmpeg", "imagemagick", "python3"}, {"gimp", "blender", "inkscape"}, {"purpose-run", "desktop-drive"}},
        {"security", "Security & Hardening", "security_guard", {"host.sys", "host.audit", "host.net"}, {"nft", "openssl", "sha256sum", "ss"}, {"sovereign-mission-control", "wireshark"}, {"audit-verify", "policy-check", "peer-export"}},
        {"governance", "Governance & Policy", "policy_authority", {"host.sys", "host.audit"}, {"sqlite3", "sha256sum"}, {"sovereign-mission-control"}, {"approve", "deny", "audit-verify"}},
        {"robotics", "Robotics & Edge", "edge_actuator", {"host.sys", "host.sensor", "host.net"}, {"python3", "ip", "udevadm"}, {"rviz2", "konsole"}, {"fleet-dispatch", "node-heartbeat"}},
        {"intelligence", "AI & Model Compute", "ai_inference_node", {"host.sys", "host.model", "host.voice"}, {"ollama", "whisper-cli", "piper", "nvidia-smi"}, {"sovereign-voice-talk", "sovereign-mission-control"}, {"chat-tools", "voice-route", "fleet-dispatch"}},
        {"automation", "Workflow Automation", "automation_runner", {"host.sys", "host.exec", "host.net"}, {"bash", "python3", "curl", "jq"}, {"sovereign-mission-control", "konsole"}, {"agent-tick", "purpose-run", "fleet-dispatch"}}
    };
    return kBlueprints;
}

const WorkspaceBlueprint* FindBlueprint(const std::string& slug) {
    for (const auto& b : StagedBlueprints()) if (b.slug == slug) return &b;
    return nullptr;
}

} // namespace sovereign::workspace
