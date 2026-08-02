#include "local_embedder.hpp"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <sstream>

namespace sovereign::semantic
{
namespace
{

constexpr std::size_t kHashDim = 64;

std::uint64_t Mix(std::uint64_t h)
{
    h ^= h >> 30;
    h *= 0xbf58476d1ce4e5b9ULL;
    h ^= h >> 27;
    h *= 0x94d049bb133111ebULL;
    h ^= h >> 31;
    return h;
}

void AddToken(Embedding& e, const std::string& token, float weight)
{
    if (token.empty())
        return;
    std::uint64_t h = 14695981039346656037ULL;
    for (unsigned char c : token)
    {
        h ^= c;
        h *= 1099511628211ULL;
    }
    h = Mix(h);
    const std::size_t idx = static_cast<std::size_t>(h % kHashDim);
    const float sign = (h & 1ULL) ? 1.0f : -1.0f;
    e.v[idx] += sign * weight;
}

std::string ShellEscape(const std::string& s)
{
    std::string o = "'";
    for (char c : s)
    {
        if (c == '\'')
            o += "'\\''";
        else
            o.push_back(c);
    }
    o.push_back('\'');
    return o;
}

std::string RunCurl(const std::string& args)
{
    const std::string cmd = "curl -sS --max-time 60 " + args + " 2>/dev/null";
    std::array<char, 4096> buffer{};
    std::string out;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        return {};
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe))
        out += buffer.data();
    pclose(pipe);
    return out;
}

// Extract JSON array of numbers after "embedding"
bool ParseEmbeddingArray(const std::string& json, Embedding& out)
{
    auto pos = json.find("\"embedding\"");
    if (pos == std::string::npos)
        return false;
    pos = json.find('[', pos);
    if (pos == std::string::npos)
        return false;
    auto end = json.find(']', pos);
    if (end == std::string::npos)
        return false;
    out.v.clear();
    std::string body = json.substr(pos + 1, end - pos - 1);
    std::stringstream ss(body);
    std::string part;
    while (std::getline(ss, part, ','))
    {
        // trim
        std::size_t a = 0;
        while (a < part.size() && (part[a] == ' ' || part[a] == '\n'))
            ++a;
        if (a >= part.size())
            continue;
        out.v.push_back(std::strtof(part.c_str() + a, nullptr));
    }
    return !out.v.empty();
}

void L2Normalize(Embedding& e)
{
    float sum = 0.0f;
    for (float x : e.v)
        sum += x * x;
    if (sum > 0.0f)
    {
        const float inv = 1.0f / std::sqrt(sum);
        for (float& x : e.v)
            x *= inv;
    }
}

} // namespace

Embedding EmbedTextHash(const std::string& text)
{
    Embedding e;
    e.v.assign(kHashDim, 0.0f);
    std::string tok;
    auto flush = [&]() {
        if (!tok.empty())
        {
            for (char& c : tok)
            {
                if (c >= 'A' && c <= 'Z')
                    c = static_cast<char>(c - 'A' + 'a');
            }
            AddToken(e, tok, 1.0f);
            tok.clear();
        }
    };
    for (char c : text)
    {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9'))
            tok.push_back(c);
        else
            flush();
    }
    flush();
    L2Normalize(e);
    return e;
}

Embedding EmbedText(const std::string& text, std::string* model_used)
{
    const char* host_env = std::getenv("OLLAMA_HOST");
    std::string base =
        (host_env && host_env[0]) ? host_env : "http://127.0.0.1:11434";
    while (!base.empty() && base.back() == '/')
        base.pop_back();

    const char* model_env = std::getenv("SOVEREIGN_EMBED_MODEL");
    std::string model =
        (model_env && model_env[0]) ? model_env : "nomic-embed-text";

    // Ollama embeddings API
    std::ostringstream body;
    body << "{\"model\":\"" << model << "\",\"prompt\":";
    // JSON-escape prompt
    body << '"';
    for (char c : text)
    {
        if (c == '"' || c == '\\')
            body << '\\';
        if (c == '\n')
            body << "\\n";
        else
            body << c;
    }
    body << "\"}";

    const std::string args =
        "-H " + ShellEscape("Content-Type: application/json") + " -d " +
        ShellEscape(body.str()) + " " + ShellEscape(base + "/api/embeddings");

    const std::string raw = RunCurl(args);
    Embedding emb;
    if (ParseEmbeddingArray(raw, emb))
    {
        L2Normalize(emb);
        if (model_used)
            *model_used = model;
        return emb;
    }

    // Offline / model-missing fallback
    if (model_used)
        *model_used = "hash-v1";
    return EmbedTextHash(text);
}

float CosineSimilarity(const Embedding& a, const Embedding& b)
{
    if (a.v.empty() || b.v.empty())
        return 0.0f;
    const std::size_t n = std::min(a.v.size(), b.v.size());
    float dot = 0.0f;
    for (std::size_t i = 0; i < n; ++i)
        dot += a.v[i] * b.v[i];
    // If dims differ (e.g. model vs hash), similarity is partial — still useful
    return dot;
}

} // namespace sovereign::semantic
