#pragma once

#include "ollama_client.hpp"

namespace sovereign::model_router
{

// Product-facing router: Ollama-only in v1 per owner decision.
class ModelRouter
{
public:
    explicit ModelRouter(std::string ollama_base = "http://127.0.0.1:11434");

    Result<bool> Health() const;
    Result<std::vector<ModelInfo>> ListModels() const;
    Result<ChatResponse> Chat(const ChatRequest& request) const;

private:
    OllamaClient client_;
};

} // namespace sovereign::model_router
