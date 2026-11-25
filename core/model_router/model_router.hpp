#pragma once

#include "ollama_client.hpp"

#include <map>
#include <string>

namespace sovereign::model_router
{

class ModelRouter
{
public:
    explicit ModelRouter(std::string ollama_base = "http://127.0.0.1:11434");

    Result<bool> Health() const;
    Result<std::vector<ModelInfo>> ListModels() const;
    Result<ChatResponse> Chat(const ChatRequest& request) const;

    void BindRoute(const std::string& scope, const std::string& model);
    std::string ResolveModel(const std::string& scope,
                             const std::string& fallback_model) const;
    const std::map<std::string, std::string>& Routes() const noexcept
    {
        return routes_;
    }

private:
    OllamaClient client_;
    std::map<std::string, std::string> routes_;
};

} // namespace sovereign::model_router
