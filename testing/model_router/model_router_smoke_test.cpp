#include "model_router.hpp"
#include "test_require.hpp"
#include <iostream>

using namespace sovereign::model_router;

int main()
{
    ModelRouter router;

    // Offline unit-style: invalid request rejected without network dependency
    ChatRequest bad;
    auto r = router.Chat(bad);
    REQUIRE(r.error == RouterError::InvalidArgument);

    ChatRequest req;
    req.model = "test-model";
    req.messages.push_back(ChatMessage{"user", "hi", {}, {}});
    ToolDefinition tool;
    tool.name = "lookup";
    tool.description = "lookup fact";
    tool.parameters_json = "{\"type\":\"object\",\"properties\":{}}";
    req.tools.push_back(tool);

    // Health may fail if Ollama not running — not a test failure for offline CI
    auto health = router.Health();
    if (health.Ok() && health.value)
    {
        auto models = router.ListModels();
        REQUIRE(models.Ok());
        std::cout << "model_router_smoke_test: OK (ollama up, models="
                  << models.value.size() << ")\n";
    }
    else
    {
        std::cout << "model_router_smoke_test: OK (offline; invalid-arg path checked)\n";
    }
    return 0;
}
