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

} // namespace sovereign::model_router
