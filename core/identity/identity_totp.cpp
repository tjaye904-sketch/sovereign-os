#include "identity_totp.hpp"

#include <array>
#include <chrono>
#include <cstring>
#include <fstream>
#include <sstream>

namespace sovereign::identity::totp
{
namespace
{

// Minimal SHA-1
struct Sha1
{
    std::uint32_t h[5]{0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    std::uint64_t total{0};
    std::uint8_t buf[64]{};
    std::size_t buf_len{0};

    static std::uint32_t rol(std::uint32_t v, int n)
    {
        return (v << n) | (v >> (32 - n));
    }

    void process(const std::uint8_t block[64])
    {
        std::uint32_t w[80];
        for (int i = 0; i < 16; ++i)
            w[i] = (std::uint32_t(block[4 * i]) << 24) | (std::uint32_t(block[4 * i + 1]) << 16) |
                   (std::uint32_t(block[4 * i + 2]) << 8) | std::uint32_t(block[4 * i + 3]);
        for (int i = 16; i < 80; ++i)
            w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
        for (int i = 0; i < 80; ++i)
        {
            std::uint32_t f, k;
            if (i < 20)
            {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            }
            else if (i < 40)
            {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            }
            else if (i < 60)
            {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            }
            else
            {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            std::uint32_t t = rol(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = rol(b, 30);
            b = a;
            a = t;
        }
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
    }

    void update(const std::uint8_t* data, std::size_t len)
    {
        total += len;
        while (len > 0)
        {
            std::size_t n = std::min(len, 64 - buf_len);
            std::memcpy(buf + buf_len, data, n);
            buf_len += n;
            data += n;
            len -= n;
            if (buf_len == 64)
            {
                process(buf);
                buf_len = 0;
            }
        }
    }

    std::array<std::uint8_t, 20> final()
    {
        std::uint64_t bit_len = total * 8;
        std::uint8_t pad = 0x80;
        update(&pad, 1);
        std::uint8_t z = 0;
        while (buf_len != 56)
            update(&z, 1);
        std::uint8_t lenb[8];
        for (int i = 0; i < 8; ++i)
            lenb[7 - i] = static_cast<std::uint8_t>((bit_len >> (8 * i)) & 0xff);
        update(lenb, 8);
        std::array<std::uint8_t, 20> out{};
        for (int i = 0; i < 5; ++i)
        {
            out[4 * i] = static_cast<std::uint8_t>((h[i] >> 24) & 0xff);
            out[4 * i + 1] = static_cast<std::uint8_t>((h[i] >> 16) & 0xff);
            out[4 * i + 2] = static_cast<std::uint8_t>((h[i] >> 8) & 0xff);
            out[4 * i + 3] = static_cast<std::uint8_t>(h[i] & 0xff);
        }
        return out;
    }
};

std::array<std::uint8_t, 20> HmacSha1(const std::vector<std::uint8_t>& key, const std::uint8_t* msg, std::size_t msg_len)
{
    std::vector<std::uint8_t> k = key;
    if (k.size() > 64)
    {
        Sha1 s;
        s.update(k.data(), k.size());
        auto dig = s.final();
        k.assign(dig.begin(), dig.end());
    }
    k.resize(64, 0);
    std::uint8_t ipad[64], opad[64];
    for (int i = 0; i < 64; ++i)
    {
        ipad[i] = static_cast<std::uint8_t>(k[i] ^ 0x36);
        opad[i] = static_cast<std::uint8_t>(k[i] ^ 0x5c);
    }
    Sha1 inner;
    inner.update(ipad, 64);
    inner.update(msg, msg_len);
    auto idig = inner.final();
    Sha1 outer;
    outer.update(opad, 64);
    outer.update(idig.data(), idig.size());
    return outer.final();
}

} // namespace

std::string Base32Encode(const std::vector<std::uint8_t>& raw)
{
    static const char* alp = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    std::string out;
    int buffer = 0, bits = 0;
    for (auto b : raw)
    {
        buffer = (buffer << 8) | b;
        bits += 8;
        while (bits >= 5)
        {
            out.push_back(alp[(buffer >> (bits - 5)) & 31]);
            bits -= 5;
        }
    }
    if (bits > 0)
        out.push_back(alp[(buffer << (5 - bits)) & 31]);
    return out;
}

std::vector<std::uint8_t> Base32Decode(const std::string& b32)
{
    std::vector<std::uint8_t> out;
    int buffer = 0, bits = 0;
    for (char c : b32)
    {
        if (c == '=' || c == ' ')
            continue;
        int v = -1;
        if (c >= 'A' && c <= 'Z')
            v = c - 'A';
        else if (c >= 'a' && c <= 'z')
            v = c - 'a';
        else if (c >= '2' && c <= '7')
            v = c - '2' + 26;
        if (v < 0)
            continue;
        buffer = (buffer << 5) | v;
        bits += 5;
        if (bits >= 8)
        {
            out.push_back(static_cast<std::uint8_t>((buffer >> (bits - 8)) & 0xff));
            bits -= 8;
        }
    }
    return out;
}

std::vector<std::uint8_t> RandomSecret(std::size_t n)
{
    std::vector<std::uint8_t> out(n);
    std::ifstream ur("/dev/urandom", std::ios::binary);
    if (ur)
        ur.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(n));
    else
        for (std::size_t i = 0; i < n; ++i)
            out[i] = static_cast<std::uint8_t>(i * 17 + 3);
    return out;
}

std::string ProvisioningUri(const std::string& b32_secret, const std::string& account)
{
    return "otpauth://totp/Sovereign:" + account + "?secret=" + b32_secret +
           "&issuer=Sovereign&algorithm=SHA1&digits=6&period=30";
}

bool VerifyCode(const std::string& b32_secret, const std::string& code, std::int64_t now_unix)
{
    if (code.size() != 6)
        return false;
    for (char c : code)
        if (c < '0' || c > '9')
            return false;
    auto key = Base32Decode(b32_secret);
    if (key.empty())
        return false;
    const std::uint64_t expected = std::stoull(code);
    for (int w = -1; w <= 1; ++w)
    {
        std::uint64_t counter = static_cast<std::uint64_t>((now_unix / 30) + w);
        std::uint8_t msg[8];
        for (int i = 7; i >= 0; --i)
        {
            msg[i] = static_cast<std::uint8_t>(counter & 0xff);
            counter >>= 8;
        }
        auto mac = HmacSha1(key, msg, 8);
        int off = mac[19] & 0x0f;
        std::uint32_t bin = ((mac[off] & 0x7f) << 24) | ((mac[off + 1] & 0xff) << 16) |
                            ((mac[off + 2] & 0xff) << 8) | (mac[off + 3] & 0xff);
        std::uint32_t otp = bin % 1000000;
        if (otp == expected)
            return true;
    }
    return false;
}

} // namespace sovereign::identity::totp
