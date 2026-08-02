# Header-only common kernel types
add_library(sovereign_common INTERFACE)
target_include_directories(sovereign_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(sovereign_common INTERFACE cxx_std_23)
