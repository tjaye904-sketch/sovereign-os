set(SOVEREIGN_TEST_COMMON_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/common)

function(sovereign_add_smoke name src_dir)
    set(src "${CMAKE_CURRENT_SOURCE_DIR}/${src_dir}/${name}.cpp")
    if(NOT EXISTS "${src}")
        message(STATUS "skip missing test ${name}")
        return()
    endif()
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE ${ARGN})
    target_include_directories(${name} PRIVATE
        ${SOVEREIGN_TEST_COMMON_INCLUDE}
        ${CMAKE_CURRENT_SOURCE_DIR}/${src_dir})
    target_compile_features(${name} PRIVATE cxx_std_23)
    target_compile_options(${name} PRIVATE -Wall -Wextra)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

if(SOVEREIGN_BUILD_EVENT_RUNTIME)
    add_subdirectory(runtime)
endif()

sovereign_add_smoke(service_orchestrator_smoke_test os_runtime sovereign_os_runtime)


sovereign_add_smoke(identity_service_smoke_test identity sovereign_identity)
sovereign_add_smoke(identity_persistence_smoke_test identity sovereign_identity)
sovereign_add_smoke(identity_acceptance_smoke_test identity sovereign_identity)
sovereign_add_smoke(identity_revocation_hardening_smoke_test identity sovereign_identity)
sovereign_add_smoke(identity_mfa_scaffold_smoke_test identity sovereign_identity)
sovereign_add_smoke(identity_totp_smoke_test identity sovereign_identity)
sovereign_add_smoke(identity_mfa_recovery_smoke_test identity sovereign_identity)
sovereign_add_smoke(identity_require_mfa_smoke_test identity sovereign_identity)
sovereign_add_smoke(identity_governance_integration_smoke_test identity sovereign_identity sovereign_governance)
sovereign_add_smoke(governance_service_smoke_test governance sovereign_governance)
sovereign_add_smoke(policy_engine_smoke_test policy sovereign_policy)
sovereign_add_smoke(memory_engine_smoke_test memory sovereign_memory)
sovereign_add_smoke(workspace_service_smoke_test workspace sovereign_workspace)
sovereign_add_smoke(archive_service_smoke_test archive sovereign_archive)
sovereign_add_smoke(sync_service_smoke_test sync sovereign_sync)
sovereign_add_smoke(model_router_smoke_test model_router sovereign_model_router)
sovereign_add_smoke(cognitive_governance_smoke_test cognitive sovereign_cognitive)
sovereign_add_smoke(cognitive_plan_enrichment_smoke_test cognitive sovereign_cognitive)
sovereign_add_smoke(action_executor_smoke_test executor sovereign_executor)
sovereign_add_smoke(sovereign_host_smoke_test product sovereign_product)

if(TARGET sovereign-cli)
    add_test(NAME cli_parse_usage COMMAND $<TARGET_FILE:sovereign-cli>)
    set_tests_properties(cli_parse_usage PROPERTIES PASS_REGULAR_EXPRESSION "sovereign-cli")
    add_test(
        NAME cli_parse_status
        COMMAND ${CMAKE_COMMAND} -E env
            "SOVEREIGN_DATA_ROOT=${CMAKE_BINARY_DIR}/ctest-data-status"
            $<TARGET_FILE:sovereign-cli> status)
endif()

if(TARGET sovereign-daemon)
    add_test(NAME daemon_parse_help COMMAND $<TARGET_FILE:sovereign-daemon> --help)
    set_tests_properties(daemon_parse_help PROPERTIES PASS_REGULAR_EXPRESSION "sovereign-daemon")
endif()
