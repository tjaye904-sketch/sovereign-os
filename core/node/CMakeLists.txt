add_library(sovereign_node STATIC node_registry.cpp)
target_include_directories(sovereign_node PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(sovereign_node PUBLIC cxx_std_23)
target_compile_options(sovereign_node PRIVATE -Wall -Wextra -Wpedantic)
