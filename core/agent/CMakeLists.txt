add_library(sovereign_agent STATIC agent_scheduler.cpp)
target_include_directories(sovereign_agent PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(sovereign_agent PUBLIC cxx_std_23)
target_compile_options(sovereign_agent PRIVATE -Wall -Wextra -Wpedantic)
