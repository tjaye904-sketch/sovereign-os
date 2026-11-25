#include "peer_auth.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <fstream>
#include <random>
#include <sstream>
#include <system_error>

namespace sovereign::federation
{

namespace
{

// ---- SHA-256 (FIPS 180-4), self-contained: no external crypto dependency ----
struct Sha256Ctx
{
    std::uint32_t h[8]{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                       0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    std::uint8_t buf[64]{};
    std::uint64_t total{0};
    std::size_t fill{0};
};

constexpr std::uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

inline std::uint32_t Rotr(std::uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

void Block(Sha256Ctx& c, const std::uint8_t* p)
{
    std::uint32_t w[64];
    for (int i = 0; i < 16; ++i)
        w[i] = (std::uint32_t(p[i * 4]) << 24) | (std::uint32_t(p[i * 4 + 1]) << 16) |
               (std::uint32_t(p[i * 4 + 2]) << 8) | std::uint32_t(p[i * 4 + 3]);
    for (int i = 16; i < 64; ++i)
    {
        const std::uint32_t s0 = Rotr(w[i - 15], 7) ^ Rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        const std::uint32_t s1 = Rotr(w[i - 2], 17) ^ Rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    std::uint32_t a = c.h[0], b = c.h[1], cc = c.h[2], d = c.h[3];
    std::uint32_t e = c.h[4], f = c.h[5], g = c.h[6], h = c.h[7];
    for (int i = 0; i < 64; ++i)
    {
        const std::uint32_t S1 = Rotr(e, 6) ^ Rotr(e, 11) ^ Rotr(e, 25);
        const std::uint32_t ch = (e & f) ^ (~e & g);
        const std::uint32_t t1 = h + S1 + ch + K[i] + w[i];
        const std::uint32_t S0 = Rotr(a, 2) ^ Rotr(a, 13) ^ Rotr(a, 22);
        const std::uint32_t mj = (a & b) ^ (a & cc) ^ (b & cc);
        const std::uint32_t t2 = S0 + mj;
        h = g; g = f; f = e; e = d + t1; d = cc; cc = b; b = a; a = t1 + t2;
    }
    c.h[0] += a; c.h[1] += b; c.h[2] += cc; c.h[3] += d;
    c.h[4] += e; c.h[5] += f; c.h[6] += g; c.h[7] += h;
}

void Update(Sha256Ctx& c, const std::uint8_t* data, std::size_t len)
{
    c.total += len;
    while (len > 0)
    {
        const std::size_t n = std::min<std::size_t>(64 - c.fill, len);
        std::memcpy(c.buf + c.fill, data, n);
        c.fill += n; data += n; len -= n;
        if (c.fill == 64) { Block(c, c.buf); c.fill = 0; }
    }
}

std::array<std::uint8_t, 32> Final(Sha256Ctx& c)
{
    const std::uint64_t bits = c.total * 8;
    const std::uint8_t pad = 0x80;
    Update(c, &pad, 1);
    const std::uint8_t zero = 0;
    while (c.fill != 56) Update(c, &zero, 1);
    std::uint8_t len[8];
    for (int i = 0; i < 8; ++i) len[i] = std::uint8_t(bits >> (56 - 8 * i));
    Update(c, len, 8);
    std::array<std::uint8_t, 32> out{};
    for (int i = 0; i < 8; ++i)
    {
        out[i * 4] = std::uint8_t(c.h[i] >> 24);
        out[i * 4 + 1] = std::uint8_t(c.h[i] >> 16);
        out[i * 4 + 2] = std::uint8_t(c.h[i] >> 8);
        out[i * 4 + 3] = std::uint8_t(c.h[i]);
    }
    return out;
}

std::array<std::uint8_t, 32> Sha256Raw(const std::string& s)
{
    Sha256Ctx c;
    Update(c, reinterpret_cast<const std::uint8_t*>(s.data()), s.size());
    return Final(c);
}

std::string ToHex(const std::uint8_t* d, std::size_t n)
{
    static const char* digits = "0123456789abcdef";
    std::string out;
    out.reserve(n * 2);
    for (std::size_t i = 0; i < n; ++i)
    {
        out.push_back(digits[d[i] >> 4]);
        out.push_back(digits[d[i] & 15]);
    }
    return out;
}

// HMAC (RFC 2104) with a raw string key.
std::array<std::uint8_t, 32> HmacRaw(const std::string& key, const std::string& msg)
{
    std::string k = key;
    if (k.size() > 64)
    {
        auto d = Sha256Raw(k);
        k.assign(reinterpret_cast<const char*>(d.data()), d.size());
    }
    k.resize(64, '\0');
    std::string ipad(64, '\0'), opad(64, '\0');
    for (int i = 0; i < 64; ++i)
    {
        ipad[i] = char(k[i] ^ 0x36);
        opad[i] = char(k[i] ^ 0x5c);
    }
    auto inner = Sha256Raw(ipad + msg);
    return Sha256Raw(opad + std::string(reinterpret_cast<const char*>(inner.data()), inner.size()));
}

bool ConstTimeEq(const std::string& a, const std::string& b)
{
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (std::size_t i = 0; i < a.size(); ++i) diff |= (unsigned char)(a[i] ^ b[i]);
    return diff == 0;
}

std::string RandomHex(std::size_t bytes)
{
    std::random_device rd;
    std::string out;
    std::vector<std::uint8_t> v(bytes);
    for (auto& b : v) b = std::uint8_t(rd());
    return ToHex(v.data(), v.size());
}

std::uint64_t NowSeconds()
{
    return std::uint64_t(std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now().time_since_epoch())
                             .count());
}

bool IsHex(const std::string& s)
{
    return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) {
        return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
    });
}

} // namespace

const char* PeerAuthErrorToString(PeerAuthError e) noexcept
{
    switch (e)
    {
        case PeerAuthError::Ok: return "ok";
        case PeerAuthError::InvalidArgument: return "invalid_argument";
        case PeerAuthError::UnknownNode: return "unknown_node";
        case PeerAuthError::AlreadyEnrolled: return "already_enrolled";
        case PeerAuthError::NotEnrolled: return "not_enrolled";
        case PeerAuthError::Revoked: return "revoked";
        case PeerAuthError::StorageFailure: return "storage_failure";
        case PeerAuthError::BadChallenge: return "bad_challenge";
        case PeerAuthError::BadResponse: return "bad_response";
    }
    return "unknown_error";
}

std::string Sha256Hex(const std::string& data)
{
    auto d = Sha256Raw(data);
    return ToHex(d.data(), d.size());
}

std::string HmacSha256Hex(const std::string& key, const std::string& message)
{
    auto d = HmacRaw(key, message);
    return ToHex(d.data(), d.size());
}

PeerAuth::PeerAuth(std::filesystem::path data_root, std::uint64_t challenge_ttl_seconds)
    : path_(std::move(data_root) / "federation" / "peers.dat"), ttl_(challenge_ttl_seconds)
{
    Load();
}

std::string PeerAuth::Message(const std::string& node, const std::string& challenge)
{
    return "sov-peer-auth-v1|" + node + "|" + challenge;
}

// Peer holds `secret`; both sides derive key = SHA256(secret). The server stores
// only that derived key (verifier), so a stolen peers.dat allows impersonation
// of enrolled peers: protect it like a credential (file is chmod 0600).
std::string PeerAuth::Respond(const std::string& secret_hex,
                              const std::string& node_id_hex,
                              const std::string& challenge_hex)
{
    return HmacSha256Hex(Sha256Hex(secret_hex), Message(node_id_hex, challenge_hex));
}

bool PeerAuth::Load()
{
    std::ifstream in(path_);
    if (!in) return true; // nothing yet
    std::string node, verifier;
    int revoked = 0;
    while (in >> node >> verifier >> revoked)
        peers_[node] = Peer{revoked != 0, verifier};
    return true;
}

bool PeerAuth::Save() const
{
    std::error_code ec;
    std::filesystem::create_directories(path_.parent_path(), ec);
    if (ec) return false;
    const auto tmp = path_.string() + ".tmp";
    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out) return false;
        for (const auto& [node, p] : peers_)
            out << node << ' ' << p.verifier_hex << ' ' << (p.revoked ? 1 : 0) << '\n';
        if (!out.good()) return false;
    }
    std::filesystem::permissions(tmp,
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
        std::filesystem::perm_options::replace, ec);
    std::filesystem::rename(tmp, path_, ec);
    return !ec;
}

Result<std::string> PeerAuth::Enroll(const std::string& node_id_hex, const node::NodeRegistry& registry)
{
    Result<std::string> r;
    if (!IsHex(node_id_hex)) { r.error = PeerAuthError::InvalidArgument; return r; }

    node::NodeId id{};
    if (!node::NodeRegistry::IdFromHex(node_id_hex, id) || !registry.Get(id).Ok())
    {
        r.error = PeerAuthError::UnknownNode;
        return r;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (peers_.count(node_id_hex)) { r.error = PeerAuthError::AlreadyEnrolled; return r; }

    const std::string secret = RandomHex(32);
    peers_[node_id_hex] = Peer{false, Sha256Hex(secret)};
    if (!Save())
    {
        peers_.erase(node_id_hex);
        r.error = PeerAuthError::StorageFailure;
        return r;
    }
    r.value = secret;
    return r;
}

Result<void> PeerAuth::Revoke(const std::string& node_id_hex)
{
    Result<void> r;
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = peers_.find(node_id_hex);
    if (it == peers_.end()) { r.error = PeerAuthError::NotEnrolled; return r; }
    it->second.revoked = true;
    pending_.erase(node_id_hex);
    if (!Save()) r.error = PeerAuthError::StorageFailure;
    return r;
}

bool PeerAuth::IsEnrolled(const std::string& node_id_hex) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = peers_.find(node_id_hex);
    return it != peers_.end() && !it->second.revoked;
}

std::vector<std::string> PeerAuth::ListEnrolled() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> out;
    for (const auto& [node, p] : peers_)
        if (!p.revoked) out.push_back(node);
    return out;
}

Result<std::string> PeerAuth::IssueChallenge(const std::string& node_id_hex)
{
    Result<std::string> r;
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = peers_.find(node_id_hex);
    if (it == peers_.end()) { r.error = PeerAuthError::NotEnrolled; return r; }
    if (it->second.revoked) { r.error = PeerAuthError::Revoked; return r; }
    Pending p{RandomHex(16), NowSeconds() + ttl_};
    r.value = p.challenge_hex;
    pending_[node_id_hex] = std::move(p);
    return r;
}

Result<void> PeerAuth::Verify(const std::string& node_id_hex,
                              const std::string& challenge_hex,
                              const std::string& response_hex)
{
    Result<void> r;
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = peers_.find(node_id_hex);
    if (it == peers_.end()) { r.error = PeerAuthError::NotEnrolled; return r; }
    if (it->second.revoked) { r.error = PeerAuthError::Revoked; return r; }

    // Single use: consume the challenge whether or not verification succeeds.
    auto pit = pending_.find(node_id_hex);
    if (pit == pending_.end()) { r.error = PeerAuthError::BadChallenge; return r; }
    const Pending pend = pit->second;
    pending_.erase(pit);
    if (NowSeconds() > pend.expires || !ConstTimeEq(pend.challenge_hex, challenge_hex))
    {
        r.error = PeerAuthError::BadChallenge;
        return r;
    }

    const std::string expect = HmacSha256Hex(it->second.verifier_hex, Message(node_id_hex, challenge_hex));
    if (!ConstTimeEq(expect, response_hex)) r.error = PeerAuthError::BadResponse;
    return r;
}

} // namespace sovereign::federation
