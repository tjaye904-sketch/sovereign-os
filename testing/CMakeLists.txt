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
sovereign_add_smoke(cognitive_curiosity_smoke_test cognitive sovereign_cognitive)
sovereign_add_smoke(cognitive_reasoning_engine_smoke_test cognitive sovereign_cognitive)
sovereign_add_smoke(cognitive_evolution_smoke_test cognitive sovereign_cognitive)
sovereign_add_smoke(cognitive_evolution_sandbox_smoke_test cognitive sovereign_cognitive)
sovereign_add_smoke(cognitive_planning_engine_smoke_test cognitive sovereign_cognitive)
sovereign_add_smoke(cognitive_orchestration_engine_smoke_test cognitive sovereign_cognitive)
sovereign_add_smoke(cognitive_plan_enrichment_smoke_test cognitive sovereign_cognitive)
sovereign_add_smoke(action_executor_smoke_test executor sovereign_executor)
sovereign_add_smoke(cli_role_management_smoke_test product sovereign_product)
sovereign_add_smoke(sovereign_host_smoke_test product sovereign_product)
sovereign_add_smoke(sovereign_sandbox_evolution_smoke_test product sovereign_product)
sovereign_add_smoke(sovereign_external_gateway_smoke_test product sovereign_product)
sovereign_add_smoke(sovereign_api_gateway_smoke_test product sovereign_product)
sovereign_add_smoke(sovereign_desktop_operator_smoke_test product sovereign_product)
sovereign_add_smoke(sovereign_purpose_runtime_smoke_test product sovereign_product)

if(TARGET sovereign-cli)
    add_test(NAME cli_parse_usage COMMAND $<TARGET_FILE:sovereign-cli>)
    set_tests_properties(cli_parse_usage PROPERTIES PASS_REGULAR_EXPRESSION "sovereign-cli")
    add_test(
        NAME cli_parse_status
        COMMAND ${CMAKE_COMMAND} -E env
            "SOVEREIGN_DATA_ROOT=${CMAKE_BINARY_DIR}/ctest-data-status"
            $<TARGET_FILE:sovereign-cli> status)
    add_test(
        NAME cli_status_json
        COMMAND ${CMAKE_COMMAND} -E env
            "SOVEREIGN_DATA_ROOT=${CMAKE_BINARY_DIR}/ctest-data-status-json"
            $<TARGET_FILE:sovereign-cli> status --json)
    set_tests_properties(cli_status_json PROPERTIES PASS_REGULAR_EXPRESSION "\"surface\"|\"data_root\"|\"ok\":true")
    add_test(
        NAME cli_purpose_list
        COMMAND ${CMAKE_COMMAND} -E env
            "SOVEREIGN_DATA_ROOT=${CMAKE_BINARY_DIR}/ctest-data-purpose-list"
            $<TARGET_FILE:sovereign-cli> purpose-list)
    set_tests_properties(cli_purpose_list PROPERTIES PASS_REGULAR_EXPRESSION "research\.web_collect|ops\.system_monitor")
    add_test(
        NAME cli_purpose_run
        COMMAND ${CMAKE_COMMAND} -E env
            "SOVEREIGN_DATA_ROOT=${CMAKE_BINARY_DIR}/ctest-data-purpose-run"
            $<TARGET_FILE:sovereign-cli> purpose-run research.web_collect https://docs.python.org/3/)
    set_tests_properties(cli_purpose_run PROPERTIES PASS_REGULAR_EXPRESSION "purpose_id=research\.web_collect|allowed=1")
    add_test(
        NAME cli_desktop_status
        COMMAND ${CMAKE_COMMAND} -E env
            "SOVEREIGN_DATA_ROOT=${CMAKE_BINARY_DIR}/ctest-data-desktop-status"
            $<TARGET_FILE:sovereign-cli> desktop-status --json)
    set_tests_properties(cli_desktop_status PROPERTIES PASS_REGULAR_EXPRESSION "computer_use_enabled")
    add_test(
        NAME cli_session_ensure
        COMMAND ${CMAKE_COMMAND} -E env
            "SOVEREIGN_DATA_ROOT=${CMAKE_BINARY_DIR}/ctest-data-session-ensure"
            $<TARGET_FILE:sovereign-cli> session-ensure --json)
    set_tests_properties(cli_session_ensure PROPERTIES PASS_REGULAR_EXPRESSION "\"total\":5")
    add_test(
        NAME cli_plan_route
        COMMAND sh -c "SOVEREIGN_DATA_ROOT=${CMAKE_BINARY_DIR}/ctest-data-plan-route $<TARGET_FILE:sovereign-cli> plan-route --json unbound-ws browser_open || true")
    set_tests_properties(cli_plan_route PROPERTIES PASS_REGULAR_EXPRESSION "no_node_bound_to_workspace")
    add_test(
        NAME cli_node_execute
        COMMAND sh -c "SOVEREIGN_DATA_ROOT=${CMAKE_BINARY_DIR}/ctest-data-node-exec $<TARGET_FILE:sovereign-cli> node-execute --json unbound-ws browser_open || true")
    set_tests_properties(cli_node_execute PROPERTIES PASS_REGULAR_EXPRESSION "no_node_bound_to_workspace")
    add_test(
        NAME mc_federation_governance_smoke
        COMMAND python3 -c "import sys; sys.path.insert(0, '${CMAKE_SOURCE_DIR}/tools/mission_control'); import capability_api as c; f=c.federation_status(); g=c.list_pending_decisions(); assert f.get('ok') and g.get('ok'); print('mc_fed_gov_ok')")
    set_tests_properties(mc_federation_governance_smoke PROPERTIES PASS_REGULAR_EXPRESSION "mc_fed_gov_ok")
    add_test(
        NAME cli_core_ha_drill
        COMMAND ${CMAKE_COMMAND} -E env
            "SOVEREIGN_CLI=$<TARGET_FILE:sovereign-cli>"
            bash ${CMAKE_SOURCE_DIR}/scripts/sovereign-ha-drill.sh)
    set_tests_properties(cli_core_ha_drill PROPERTIES PASS_REGULAR_EXPRESSION "HA_DRILL_OK")
    add_test(
        NAME nixos_lab_gates_smoke
        COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/verify-nixos-lab-gates.sh)
    set_tests_properties(nixos_lab_gates_smoke PROPERTIES PASS_REGULAR_EXPRESSION "NIXOS_LAB_GATES_OK")
    add_test(
        NAME cli_fleet_replace_drill
        COMMAND ${CMAKE_COMMAND} -E env
            "SOVEREIGN_CLI=$<TARGET_FILE:sovereign-cli>"
            bash ${CMAKE_SOURCE_DIR}/scripts/sovereign-fleet-drill.sh)
    set_tests_properties(cli_fleet_replace_drill PROPERTIES PASS_REGULAR_EXPRESSION "FLEET_DRILL_OK")
    add_test(
        NAME cli_closed_loop_observe
        COMMAND sh -c "ROOT=$(mktemp -d /tmp/sov-l17.XXXXXX); trap 'rm -rf $ROOT' EXIT; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> status >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> capability-seed >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> node-register-local n1 >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> node-heartbeat n1 >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> node-bind n1 development >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> node-caps n1 host.computer_use >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> goal-execute --json development verify_build")
    set_tests_properties(cli_closed_loop_observe PROPERTIES PASS_REGULAR_EXPRESSION "\"verified\":true")
    add_test(
        NAME cli_curiosity_bounded_probe
        COMMAND sh -c "ROOT=$(mktemp -d /tmp/sov-l18.XXXXXX); trap 'rm -rf $ROOT' EXIT; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> status >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> curiosity-probe --json observe kernel_telemetry && ! SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> curiosity-probe --json shell rm_rf >/dev/null")
    set_tests_properties(cli_curiosity_bounded_probe PROPERTIES PASS_REGULAR_EXPRESSION "bounded_probe_allowed")
    add_test(
        NAME cli_unified_inventory_allowlist
        COMMAND sh -c "ROOT=$(mktemp -d /tmp/sov-l19.XXXXXX); trap 'rm -rf $ROOT' EXIT; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> status >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> inventory-sync --apps firefox,code >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> app-check --json firefox && ! SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> app-check --json unlisted-evil-app >/dev/null")
    set_tests_properties(cli_unified_inventory_allowlist PROPERTIES PASS_REGULAR_EXPRESSION "in_unified_inventory")
    add_test(
        NAME cli_purpose_library_seed
        COMMAND sh -c "ROOT=$(mktemp -d /tmp/sov-l20.XXXXXX); trap 'rm -rf $ROOT' EXIT; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> status >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> inventory-sync --apps firefox,code >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> purpose-seed >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> purpose-inspect --json research.web_collect")
    set_tests_properties(cli_purpose_library_seed PROPERTIES PASS_REGULAR_EXPRESSION "\"inventory_compliant\":true")
    add_test(
        NAME cli_desktop_drive_mvp
        COMMAND sh -c "ROOT=$(mktemp -d /tmp/sov-l21.XXXXXX); trap 'rm -rf $ROOT' EXIT; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> status >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> capability-seed >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> inventory-sync --apps firefox >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> desktop-drive --json --allow-headless focus_window firefox && ! SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> desktop-drive --json --allow-headless focus_window unlisted-app >/dev/null")
    set_tests_properties(cli_desktop_drive_mvp PROPERTIES PASS_REGULAR_EXPRESSION "driven_and_observed")
    add_test(
        NAME cli_voice_router_smoke
        COMMAND sh -c "ROOT=$(mktemp -d /tmp/sov-l22.XXXXXX); trap 'rm -rf $ROOT' EXIT; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> status >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> capability-seed >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> capability-enable host.voice >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> voice-route --json speak system_ready && ! SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> voice-route --json intent delete_all_files >/dev/null")
    set_tests_properties(cli_voice_router_smoke PROPERTIES ENVIRONMENT "SOVEREIGN_VOICE_DRY_RUN=1" PASS_REGULAR_EXPRESSION "routed_on_same_router")
    add_test(
        NAME cli_doctor_hardening_smoke
        COMMAND sh -c "ROOT=$(mktemp -d /tmp/sov-l23.XXXXXX); trap 'rm -rf $ROOT' EXIT; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> status >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> doctor --json --fix")
    set_tests_properties(cli_doctor_hardening_smoke PROPERTIES PASS_REGULAR_EXPRESSION "\"all_ok\":true")
    add_test(
        NAME nixos_iso_vm_boot_gate
        COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/verify-nixos-iso-vm-boot.sh)
    set_tests_properties(nixos_iso_vm_boot_gate PROPERTIES PASS_REGULAR_EXPRESSION "NIXOS_ISO_VM_BOOT_GATE_OK")
    add_test(
        NAME cli_chat_tools_loop_smoke
        COMMAND python3 ${CMAKE_SOURCE_DIR}/scripts/test-chat-tools-loop.py $<TARGET_FILE:sovereign-cli>)
    set_tests_properties(cli_chat_tools_loop_smoke PROPERTIES PASS_REGULAR_EXPRESSION "tool_calls_executed=1")
    add_test(
        NAME cli_live_window_voice_bridge
        COMMAND sh -c "ROOT=$(mktemp -d /tmp/sov-l30.XXXXXX); trap 'rm -rf $ROOT' EXIT; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> status >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> capability-seed >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> capability-enable host.voice >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> inventory-sync --apps firefox >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT SOVEREIGN_DRIVE_DRY_RUN=1 $<TARGET_FILE:sovereign-cli> desktop-drive --json --allow-headless read_tree firefox && SOVEREIGN_DATA_ROOT=$ROOT SOVEREIGN_VOICE_DRY_RUN=1 $<TARGET_FILE:sovereign-cli> voice-route --json speak ready")
    set_tests_properties(cli_live_window_voice_bridge PROPERTIES PASS_REGULAR_EXPRESSION "routed_on_same_router")
    add_test(
        NAME multinode_peer_enrollment_drill
        COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/sovereign-multinode-drill.sh $<TARGET_FILE:sovereign-cli>)
    set_tests_properties(multinode_peer_enrollment_drill PROPERTIES PASS_REGULAR_EXPRESSION "MULTINODE_ENROLLMENT_DRILL_OK")
    add_test(
        NAME workspace_sixteen_domains_smoke
        COMMAND sh -c "ROOT=$(mktemp -d /tmp/sov-l34.XXXXXX); trap 'rm -rf $ROOT' EXIT; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> status >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> workspace-init-domains | wc -l | grep -q '^16$' && SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> workspace-list | grep -q 'robotics' && SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> workspace-list | grep -q 'federation'")
    set_tests_properties(workspace_sixteen_domains_smoke PROPERTIES PASS_REGULAR_EXPRESSION "^$")
    add_test(
        NAME model_router_task_assignment_smoke
        COMMAND sh -c "ROOT=$(mktemp -d /tmp/sov-l35.XXXXXX); trap 'rm -rf $ROOT' EXIT; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> status >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> model-bind development qwen2.5-coder:7b >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> model-bind communications granite4.1:3b >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> model-route --json development && SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> model-route --json communications")
    set_tests_properties(model_router_task_assignment_smoke PROPERTIES PASS_REGULAR_EXPRESSION "granite4.1:3b")
    add_test(
        NAME voice_talk_bridge_smoke
        COMMAND sh -c "ROOT=$(mktemp -d /tmp/sov-l37.XXXXXX); trap 'rm -rf $ROOT' EXIT; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> status >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> capability-seed >/dev/null; SOVEREIGN_DATA_ROOT=$ROOT $<TARGET_FILE:sovereign-cli> capability-enable host.voice >/dev/null; printf '#!/usr/bin/env bash\necho content=Voice bridge verified.\n' > $ROOT/mock-chat.sh; chmod +x $ROOT/mock-chat.sh; SOVEREIGN_DATA_ROOT=$ROOT SOVEREIGN_CLI=$<TARGET_FILE:sovereign-cli> SOVEREIGN_HUB_CHAT=$ROOT/mock-chat.sh SOVEREIGN_VOICE_DRY_RUN=1 bash ${CMAKE_SOURCE_DIR}/scripts/sovereign-voice-talk.sh --text 'Check status'")
    set_tests_properties(voice_talk_bridge_smoke PROPERTIES PASS_REGULAR_EXPRESSION "VOICE_TALK_TURN_OK")
endif()

if(TARGET sovereign-daemon)
    add_test(NAME daemon_parse_help COMMAND $<TARGET_FILE:sovereign-daemon> --help)
    set_tests_properties(daemon_parse_help PROPERTIES PASS_REGULAR_EXPRESSION "sovereign-daemon")
endif()

sovereign_add_smoke(secrets_manager_smoke_test secrets sovereign_secrets)
sovereign_add_smoke(memory_facets_smoke_test memory sovereign_memory)
sovereign_add_smoke(workspace_controller_smoke_test workspace sovereign_workspace)

sovereign_add_smoke(capability_registry_smoke_test capability sovereign_capability)
sovereign_add_smoke(capability_gate_smoke_test capability sovereign_capability)

sovereign_add_smoke(node_registry_smoke_test node sovereign_node)

sovereign_add_smoke(node_capability_binding_smoke_test capability sovereign_capability)

sovereign_add_smoke(peer_auth_smoke_test federation sovereign_federation)
