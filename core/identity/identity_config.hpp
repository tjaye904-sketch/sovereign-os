#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace sovereign::identity
{

struct IdentityConfig
{
    // IQ-3D: default 12 hours
    std::uint64_t session_lifetime_seconds{12ULL * 60ULL * 60ULL};

    // IQ-5D: SOVEREIGN_DATA_ROOT/identity or XDG fallback
    std::filesystem::path data_directory;
};

// Resolves data directory per SPEC IQ-5D.
IdentityConfig DefaultIdentityConfig();

} // namespace sovereign::identity
