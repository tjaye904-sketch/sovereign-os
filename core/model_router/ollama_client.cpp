#include "ollama_client.hpp"

#include <array>
#include <cstdio>
#include <memory>
#include <sstream>

namespace sovereign::model_router
{
namespace
{

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

Result<std::string> RunCurl(const std::string& args)
{
    Result<std::string> result;
    const std::string cmd = "curl -sS --max-time 25 --connect-timeout 3 " + args + " 2>/dev/null";
    std::array<char, 4096> buffer{};
    std::string out;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        result.error = RouterError::TransportFailure;
        return result;
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
    {
        out += buffer.data();
    }
    const int rc = pclose(pipe);
    if (rc != 0 && out.empty())
    {
        result.error = RouterError::TransportFailure;
        return result;
    }
    result.value = out;
    return result;
}

// Minimal JSON string extract helpers (not a full parser).
std::string ExtractStringField(const std::string& json, const std::string& key)
{
    const std::string pat = "\"" + key + "\"";
    auto pos = json.find(pat);
    if (pos == std::string::npos)
    {
        return {};
    }
    pos = json.find(':', pos + pat.size());
    if (pos == std::string::npos)
    {
        return {};
    }
    pos = json.find('"', pos + 1);
    if (pos == std::string::npos)
    {
        return {};
    }
    std::string out;
    for (std::size_t i = pos + 1; i < json.size(); ++i)
    {
        if (json[i] == '\\' && i + 1 < json.size())
        {
            out.push_back(json[++i]);
            continue;
        }
        if (json[i] == '"')
        {
            break;
        }
        out.push_back(json[i]);
    }
    return out;
}

std::string BuildToolsJson(const std::vector<ToolDefinition>& tools)
{
    if (tools.empty())
    {
        return "[]";
    }
    std::ostringstream oss;
    oss << '[';
    for (std::size_t i = 0; i < tools.size(); ++i)
    {
        if (i)
        {
            oss << ',';
        }
        const auto& t = tools[i];
        oss << "{\"type\":\"function\",\"function\":{"
            << "\"name\":\"" << t.name << "\","
            << "\"description\":\"" << t.description << "\","
            << "\"parameters\":"
            << (t.parameters_json.empty() ? "{}" : t.parameters_json)
            << "}}";
    }
    oss << ']';
    return oss.str();
}

std::string BuildMessagesJson(const std::vector<ChatMessage>& messages)
{
    std::ostringstream oss;
    oss << '[';
    for (std::size_t i = 0; i < messages.size(); ++i)
    {
        if (i)
        {
            oss << ',';
        }
        const auto& m = messages[i];
        oss << "{\"role\":\"" << m.role << "\",\"content\":\"";
        for (char c : m.content)
        {
            if (c == '"' || c == '\\')
            {
                oss << '\\';
            }
            if (c == '\n')
            {
                oss << "\\n";
            }
            else
            {
                oss << c;
            }
        }
        oss << "\"}";
    }
    oss << ']';
    return oss.str();
}

} // namespace

OllamaClient::OllamaClient(std::string base_url)
    : base_url_(std::move(base_url))
{
    while (!base_url_.empty() && base_url_.back() == '/')
    {
        base_url_.pop_back();
    }
}

Result<std::string> OllamaClient::HttpGet(const std::string& path) const
{
    return RunCurl(ShellEscape(base_url_ + path));
}

Result<std::string> OllamaClient::HttpPost(
    const std::string& path,
    const std::string& body) const
{
    const std::string args =
        "-H " + ShellEscape("Content-Type: application/json") +
        " -d " + ShellEscape(body) +
        " " + ShellEscape(base_url_ + path);
    return RunCurl(args);
}

Result<bool> OllamaClient::Health() const
{
    Result<bool> result;
    auto tags = HttpGet("/api/tags");
    if (!tags.Ok())
    {
        result.error = tags.error;
        result.value = false;
        return result;
    }
    result.value = !tags.value.empty();
    return result;
}

Result<std::vector<ModelInfo>> OllamaClient::ListModels() const
{
    Result<std::vector<ModelInfo>> result;
    auto body = HttpGet("/api/tags");
    if (!body.Ok())
    {
        result.error = body.error;
        return result;
    }
    // Best-effort: find "name":"..." occurrences
    std::string json = body.value;
    std::size_t pos = 0;
    while ((pos = json.find("\"name\"", pos)) != std::string::npos)
    {
        ModelInfo info;
        info.name = ExtractStringField(json.substr(pos), "name");
        if (!info.name.empty())
        {
            result.value.push_back(info);
        }
        pos += 6;
    }
    return result;
}

Result<ChatResponse> OllamaClient::Chat(const ChatRequest& request) const
{
    Result<ChatResponse> result;
    if (request.model.empty() || request.messages.empty())
    {
        result.error = RouterError::InvalidArgument;
        return result;
    }

    std::ostringstream body;
    body << "{\"model\":\"" << request.model << "\","
         << "\"stream\":false,"
         << "\"messages\":" << BuildMessagesJson(request.messages);
    if (!request.tools.empty())
    {
        body << ",\"tools\":" << BuildToolsJson(request.tools);
    }
    if (request.temperature >= 0.0 || request.num_predict >= 0)
    {
        body << ",\"options\":{";
        bool first = true;
        if (request.temperature >= 0.0)
        {
            body << "\"temperature\":" << request.temperature;
            first = false;
        }
        if (request.num_predict >= 0)
        {
            if (!first)
                body << ',';
            body << "\"num_predict\":" << request.num_predict;
        }
        body << '}';
    }
    body << '}';

    auto raw = HttpPost("/api/chat", body.str());
    if (!raw.Ok())
    {
        result.error = raw.error;
        result.value.error = "transport failure";
        return result;
    }

    ChatResponse response;
    response.raw_json = raw.value;
    response.model = request.model;
    response.content = ExtractStringField(raw.value, "content");
    // Tool calls: look for "tool_calls" array entries with function name
    std::size_t pos = 0;
    while ((pos = raw.value.find("\"function\"", pos)) != std::string::npos)
    {
        ToolCall tc;
        tc.name = ExtractStringField(raw.value.substr(pos), "name");
        tc.arguments_json = ExtractStringField(raw.value.substr(pos), "arguments");
        // id may appear before function
        auto id_pos = raw.value.rfind("\"id\"", pos);
        if (id_pos != std::string::npos && pos - id_pos < 200)
        {
            tc.id = ExtractStringField(raw.value.substr(id_pos), "id");
        }
        if (!tc.name.empty())
        {
            response.tool_calls.push_back(tc);
        }
        pos += 10;
    }
    response.ok = true;
    result.value = response;
    return result;
}

} // namespace sovereign::model_router
