#pragma once

// Always-on test check. Unlike assert(), never stripped by NDEBUG.
#include <cstdlib>
#include <iostream>

#define REQUIRE(expr) \
    do { \
        if (!(expr)) { \
            std::cerr << "REQUIRE failed: " << #expr << " at " << __FILE__ << ":" << __LINE__ << "\\n"; \
            std::exit(1); \
        } \
    } while (0)
