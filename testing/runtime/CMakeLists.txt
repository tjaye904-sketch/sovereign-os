# Foundational runtime smoke tests (trace-migration chains quarantined).
set(RUNTIME_SMOKES
    event_runtime_smoke_test
    scheduler_smoke_test
    lifecycle_runtime_smoke_test
    logging_runtime_smoke_test
    diagnostics_runtime_smoke_test
    configuration_runtime_smoke_test
    identity_runtime_smoke_test
    dead_letter_runtime_smoke_test
    dead_letter_replay_smoke_test
    event_retry_policy_smoke_test
    event_routing_policy_smoke_test
    event_classification_smoke_test
    event_delivery_semantics_smoke_test
    event_traceability_smoke_test
    service_runtime_smoke_test
    runtime_coordinator_smoke_test
)

foreach(t ${RUNTIME_SMOKES})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${t}.cpp")
        add_executable(${t} ${t}.cpp)
        target_link_libraries(${t} PRIVATE sovereign_event_runtime)
        target_include_directories(${t} PRIVATE
            ${CMAKE_SOURCE_DIR}/testing/common
            ${CMAKE_CURRENT_SOURCE_DIR})
        target_compile_features(${t} PRIVATE cxx_std_23)
        add_test(NAME ${t} COMMAND ${t})
    endif()
endforeach()
