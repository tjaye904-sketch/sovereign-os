#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::model_router
{

struct ToolParameter
{
    std::string name;
    std::string type;
    std::string description;
};

struct ToolDefinition
{
    std::string name;
    std::string description;
    std::string parameters_json; // JSON object schema body
};

struct ChatMessage
{
    std::string role; // system, user, assistant, tool
    std::string content;
    std::string tool_name; // when role == tool
    std::string tool_call_id;
};

struct ToolCall
{
    std::string id;
    std::string name;
    std::string arguments_json;
};

struct ChatRequest
{
    std::string model;
    std::vector<ChatMessage> messages;
    std::vector<ToolDefinition> tools;
    bool stream{false};
    double temperature{-1.0}; // <0 means omit (server default)
    int num_predict{-1};      // <0 means omit
};

struct ChatResponse
{
    std::string model;
    std::string content;
    std::vector<ToolCall> tool_calls;
    std::string raw_json;
    bool ok{false};
    std::string error;
};

struct ModelInfo
{
    std::string name;
    std::string digest;
    std::uint64_t size{0};
};

enum class RouterError : std::uint8_t
{
    Ok = 0,
    InvalidArgument,
    TransportFailure,
    ProtocolError,
    Unavailable
};

template <typename T>
struct Result
{
    RouterError error{RouterError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == RouterError::Ok;
    }
};

} // namespace sovereign::model_router
