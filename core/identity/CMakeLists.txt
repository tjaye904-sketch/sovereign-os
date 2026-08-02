add_library(
    sovereign_identity
    STATIC
    identity_config.cpp
    identity_store_memory.cpp
    identity_store_file.cpp
    identity_service.cpp
    identity_verifier.cpp
)

target_include_directories(
    sovereign_identity
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(
    sovereign_identity
    PUBLIC
        cxx_std_23
)

target_compile_options(
    sovereign_identity
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

target_link_libraries(sovereign_identity PUBLIC sovereign_common)

# Portable argon2: prefer find_library, then common distro paths, then -largon2
find_library(ARGON2_LIBRARY NAMES argon2 libargon2)
if(ARGON2_LIBRARY)
  target_link_libraries(sovereign_identity PUBLIC ${ARGON2_LIBRARY})
elseif(EXISTS /usr/lib/x86_64-linux-gnu/libargon2.so.1)
  target_link_libraries(sovereign_identity PUBLIC /usr/lib/x86_64-linux-gnu/libargon2.so.1)
else()
  target_link_libraries(sovereign_identity PUBLIC argon2)
endif()
