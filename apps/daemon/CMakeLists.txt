add_executable(sovereign-daemon main.cpp)
target_link_libraries(sovereign-daemon PRIVATE sovereign_product)
target_compile_features(sovereign-daemon PRIVATE cxx_std_23)
target_compile_options(sovereign-daemon PRIVATE -Wall -Wextra -Wpedantic)
