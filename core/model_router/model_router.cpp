#include "model_router.hpp"

namespace sovereign::model_router
{

ModelRouter::ModelRouter(std::string ollama_base)
    : client_(std::move(ollama_base))
{
}

Result<bool> ModelRouter::Health() const
{
    return client_.Health();
}

Result<std::vector<ModelInfo>> ModelRouter::ListModels() const
{
    return client_.ListModels();
}

Result<ChatResponse> ModelRouter::Chat(const ChatRequest& request) const
{
    return client_.Chat(request);
}

void ModelRouter::BindRoute(const std::string& scope, const std::string& model)
{
    if (!scope.empty() && !model.empty())
        routes_[scope] = model;
}

std::string ModelRouter::ResolveModel(const std::string& scope,
                                      const std::string& fallback_model) const
{
    const auto it = routes_.find(scope);
    if (it != routes_.end() && !it->second.empty())
        return it->second;
    if (scope == "development" || scope == "robotics" || scope == "infrastructure")
        return fallback_model;
    if (scope == "research" || scope == "intelligence")
        return fallback_model;
    return fallback_model;
}

} // namespace sovereign::model_router
