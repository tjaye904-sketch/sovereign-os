cmake_minimum_required(VERSION 3.28)

project(
    Sovereign
    VERSION 0.1.0
    DESCRIPTION "Sovereign Runtime"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SOVEREIGN_BUILD_TESTS "Build Sovereign tests" ON)

add_subdirectory(core/common)
add_subdirectory(core/os_runtime)
add_subdirectory(core/audit)
add_subdirectory(core/secrets)
add_subdirectory(core/capability)
add_subdirectory(core/node)
add_subdirectory(core/federation)
add_subdirectory(core/goals)
add_subdirectory(core/briefing)
add_subdirectory(core/agent)
add_subdirectory(core/semantic)
add_subdirectory(core/domain)
option(SOVEREIGN_BUILD_EVENT_RUNTIME
    "Build experimental core/runtime event bus (not on v1 product path; quarantined by default)"
    OFF)
if(SOVEREIGN_BUILD_EVENT_RUNTIME)
    add_subdirectory(core/runtime)
endif()
add_subdirectory(core/identity)
add_subdirectory(core/governance)
add_subdirectory(core/policy)
add_subdirectory(core/memory)
add_subdirectory(core/workspace)
add_subdirectory(core/archive)
add_subdirectory(core/model_router)
add_subdirectory(core/cognitive)
add_subdirectory(core/executor)
add_subdirectory(core/sync)
add_subdirectory(core/product)
add_subdirectory(core/diagnostics)

add_subdirectory(apps/cli)
add_subdirectory(apps/daemon)

if(SOVEREIGN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(testing)
endif()
