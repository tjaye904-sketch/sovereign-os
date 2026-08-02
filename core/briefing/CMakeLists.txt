add_library(sovereign_briefing STATIC briefing_service.cpp)
target_include_directories(sovereign_briefing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(sovereign_briefing PUBLIC cxx_std_23)
target_link_libraries(
    sovereign_briefing
    PUBLIC
        sovereign_goals
        sovereign_governance
        sovereign_memory
        sovereign_workspace
        sovereign_audit
)
target_compile_options(sovereign_briefing PRIVATE -Wall -Wextra -Wpedantic)
