#pragma once
#include "capability_registry.hpp"

namespace sovereign::capability
{

// L3 — ensure host automation capabilities exist (idempotent).
inline Result<int> EnsureHostCapabilitySeeds(CapabilityRegistry& reg)
{
    struct Item
    {
        const char* id;
        const char* desc;
    };
    static const Item kItems[] = {
        {"host.execute", "Governed action execute"},
        {"host.purpose", "Purpose-driven host automation"},
        {"host.computer_use", "Browser and computer-use"},
        {"host.sys", "System app open / desktop"},
        {"host.voice", "Speak and listen"},
        {"model.chat", "Model chat and plan"},
        {"memory.write", "Memory write"},
    };
    int added = 0;
    for (const auto& it : kItems)
    {
        auto en = reg.IsEnabled(it.id);
        if (!en.Ok())
        {
            // Not registered — SeedDefaults should have created them; try SetEnabled no-op
            // Register via SeedDefaults path: call SeedDefaults then enable
            continue;
        }
        (void)it;
    }
    // Prefer SeedDefaults then enable all host.*
    auto seed = reg.SeedDefaults();
    if (seed.Ok())
        added += static_cast<int>(seed.value);
    for (const auto& it : kItems)
    {
        auto r = reg.SetEnabled(it.id, true);
        if (r.Ok())
            ++added;
    }
    Result<int> out;
    out.value = added;
    return out;
}

} // namespace sovereign::capability
