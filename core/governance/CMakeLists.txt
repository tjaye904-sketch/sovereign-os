add_library(
    sovereign_governance
    STATIC
    governance_service.cpp
)
target_include_directories(sovereign_governance PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(sovereign_governance PUBLIC cxx_std_23)
target_compile_options(sovereign_governance PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(sovereign_governance PUBLIC sovereign_common)
