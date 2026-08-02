#include "model_router.hpp"
#include "test_require.hpp"
#include <cstdlib>
#include <iostream>

using namespace sovereign::model_router;

int main()
{
    const char* base = std::getenv("OLLAMA_HOST");
    ModelRouter router(base && base[0] ? base : "http://127.0.0.1:11434");

    auto health = router.Health();
    if (!health.Ok() || !health.value)
    {
        std::cout << "model_router_live_smoke_test: SKIP (ollama unreachable from this process)\n";
        return 0; // soft pass — sandbox may not share host network
    }

    auto models = router.ListModels();
    REQUIRE(models.Ok());

    const char* preferred = std::getenv("SOVEREIGN_OLLAMA_MODEL");
    std::string model = preferred && preferred[0]
                            ? preferred
                            : "openclaw";

    // If preferred not installed, use first listed model
    bool found = false;
    for (const auto& m : models.value)
    {
        if (m.name.find(model) != std::string::npos || model.find(m.name) != std::string::npos)
        {
            model = m.name;
            found = true;
            break;
        }
    }
    if (!found && !models.value.empty())
    {
        model = models.value.front().name;
    }
    REQUIRE(!model.empty());

    ChatRequest req;
    req.model = model;
    req.messages.push_back(ChatMessage{"user", "Reply with exactly PONG", {}, {}});
    auto chat = router.Chat(req);
    REQUIRE(chat.Ok());
    REQUIRE(chat.value.ok);
    std::cout << "model_router_live_smoke_test: OK model=" << model
              << " content_len=" << chat.value.content.size() << "\n";
    return 0;
}
