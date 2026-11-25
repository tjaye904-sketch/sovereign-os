#pragma once

// L7 — Federation peer authentication scaffold.
// Local-only: no networking. A peer can only be enrolled if its node id is in
// the NodeRegistry. Auth is challenge/response (HMAC-SHA256); the server stores
// only a verifier (SHA-256 of the secret), never the raw secret. Secrets are
// shown once at enrollment and must never be placed in peer bundles.

#include "node_registry.hpp"

#include <cstdint>
#include <filesystem>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace sovereign::federation
{

enum class PeerAuthError : std::uint8_t
{
    Ok = 0,
    InvalidArgument,
    UnknownNode,
    AlreadyEnrolled,
    NotEnrolled,
    Revoked,
    StorageFailure,
    BadChallenge,
    BadResponse
};

template <typename T>
struct Result
{
    PeerAuthError error{PeerAuthError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept { return error == PeerAuthError::Ok; }
};

template <>
struct Result<void>
{
    PeerAuthError error{PeerAuthError::Ok};
    [[nodiscard]] bool Ok() const noexcept { return error == PeerAuthError::Ok; }
};

const char* PeerAuthErrorToString(PeerAuthError e) noexcept;

// Exposed for tests / peer-side tooling.
std::string Sha256Hex(const std::string& data);
std::string HmacSha256Hex(const std::string& key, const std::string& message);

class PeerAuth
{
public:
    explicit PeerAuth(std::filesystem::path data_root, std::uint64_t challenge_ttl_seconds = 60);

    // Enroll a node known to `registry`. Returns the one-time secret (hex).
    Result<std::string> Enroll(const std::string& node_id_hex, const node::NodeRegistry& registry);
    Result<void> Revoke(const std::string& node_id_hex);
    [[nodiscard]] bool IsEnrolled(const std::string& node_id_hex) const;
    [[nodiscard]] std::vector<std::string> ListEnrolled() const;

    // Server side: single-use challenge, then verify the peer's response.
    Result<std::string> IssueChallenge(const std::string& node_id_hex);
    Result<void> Verify(const std::string& node_id_hex,
                        const std::string& challenge_hex,
                        const std::string& response_hex);

    // Peer side: compute the response from the secret received at enrollment.
    static std::string Respond(const std::string& secret_hex,
                               const std::string& node_id_hex,
                               const std::string& challenge_hex);

private:
    struct Peer
    {
        bool revoked{false};
        std::string verifier_hex;
    };
    struct Pending
    {
        std::string challenge_hex;
        std::uint64_t expires{0};
    };

    bool Load();
    bool Save() const;
    static std::string Message(const std::string& node, const std::string& challenge);

    mutable std::mutex mutex_;
    std::filesystem::path path_;
    std::uint64_t ttl_;
    std::map<std::string, Peer> peers_;
    std::map<std::string, Pending> pending_;
};

} // namespace sovereign::federation
