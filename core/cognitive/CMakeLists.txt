add_library(sovereign_cognitive STATIC
    cognitive_service.cpp
    curiosity_engine.cpp
    decision_engine.cpp
    evolution_engine.cpp
    orchestration_engine.cpp
    planning_engine.cpp
    reasoning_engine.cpp
    reflection_engine.cpp)
target_include_directories(sovereign_cognitive PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(sovereign_cognitive PUBLIC cxx_std_23)
target_link_libraries(
    sovereign_cognitive
    PUBLIC
        sovereign_common
        sovereign_governance
        sovereign_memory
        sovereign_model_router
)
target_compile_options(sovereign_cognitive PRIVATE -Wall -Wextra -Wpedantic)
