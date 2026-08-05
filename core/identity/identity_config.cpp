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
        config.data_directory = std::filesystem::path(root) / "identity";
    }
    else if (const char* home = std::getenv("HOME");
             home != nullptr && home[0] != '\0')
    {
        config.data_directory = std::filesystem::path(home) / ".local" /
                                "share" / "sovereign" / "identity";
    }
    else
    {
        config.data_directory = std::filesystem::path("/tmp/sovereign/identity");
    }

    if (const char* e = std::getenv("SOVEREIGN_REQUIRE_MFA");
        e != nullptr && e[0] != '\0' && e[0] != '0' && e[0] != 'f' &&
        e[0] != 'F' && e[0] != 'n' && e[0] != 'N')
    {
        config.require_mfa_on_login = true;
    }

    return config;
}

} // namespace sovereign::identity
