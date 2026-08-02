#pragma once

#include "model_router_types.hpp"

#include <string>

namespace sovereign::model_router
{

class OllamaClient
{
public:
    explicit OllamaClient(std::string base_url = "http://127.0.0.1:11434");

    Result<bool> Health() const;
    Result<std::vector<ModelInfo>> ListModels() const;
    Result<ChatResponse> Chat(const ChatRequest& request) const;

    const std::string& BaseUrl() const noexcept { return base_url_; }

private:
    Result<std::string> HttpPost(const std::string& path, const std::string& body) const;
    Result<std::string> HttpGet(const std::string& path) const;

    std::string base_url_;
};

} // namespace sovereign::model_router
