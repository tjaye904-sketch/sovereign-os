add_library(sovereign_product STATIC sovereign_host.cpp api_gateway.cpp external_gateway.cpp desktop_operator.cpp purpose_runtime.cpp)
target_include_directories(sovereign_product PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(sovereign_product PUBLIC cxx_std_23)
target_link_libraries(
    sovereign_product
    PUBLIC
        sovereign_audit
        sovereign_goals
        sovereign_briefing
        sovereign_agent
        sovereign_semantic
        sovereign_domain
        sovereign_identity
        sovereign_governance
        sovereign_policy
        sovereign_memory
        sovereign_workspace
        sovereign_archive
        sovereign_sync
        sovereign_model_router
        sovereign_cognitive
        sovereign_executor
)
target_compile_options(sovereign_product PRIVATE -Wall -Wextra -Wpedantic)
