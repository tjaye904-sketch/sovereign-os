add_executable(sovereign-cli main.cpp)
target_link_libraries(
sovereign-cli PRIVATE sovereign_product sovereign_diagnostics)
target_compile_features(sovereign-cli PRIVATE cxx_std_23)
target_compile_options(sovereign-cli PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(sovereign-cli PRIVATE sovereign_secrets)
