#pragma once
// Always-on test assertion — never compiled out under NDEBUG.
#include <cstdlib>
#include <iostream>
#include <string_view>

inline void SovereignRequire(
    bool condition,
    std::string_view expr,
    std::string_view file,
    int line)
{
    if (!condition)
    {
        std::cerr << "REQUIRE failed: " << expr << " at " << file << ':'
                  << line << '\n';
        std::abort();
    }
}

#define REQUIRE(cond) \
    ::SovereignRequire(static_cast<bool>(cond), #cond, __FILE__, __LINE__)
