#include "identity_config.hpp"

#include <cstdlib>

namespace sovereign::identity
{

IdentityConfig DefaultIdentityConfig()
{
    IdentityConfig config;

    if (const char* root = std::getenv("SOVEREIGN_DATA_ROOT");
        root != nullptr && root[0] != '\0')
    {
        config.data_directory =
            std::filesystem::path(root) / "identity";
        return config;
    }

    const char* home = std::getenv("HOME");
    if (home != nullptr && home[0] != '\0')
    {
        config.data_directory =
            std::filesystem::path(home) / ".local" / "share" /
            "sovereign" / "identity";
        return config;
    }

    config.data_directory =
        std::filesystem::path("/tmp/sovereign/identity");
    return config;
}

} // namespace sovereign::identity
