add_library(sovereign_secrets STATIC secret_manager.cpp)
target_include_directories(sovereign_secrets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(sovereign_secrets PUBLIC cxx_std_23)
target_compile_options(sovereign_secrets PRIVATE -Wall -Wextra -Wpedantic)
