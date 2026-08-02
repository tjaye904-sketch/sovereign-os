#include "identity_service.hpp"
#include "identity_store_file.hpp"
#include "test_require.hpp"

#include <filesystem>
#include <iostream>

using namespace sovereign::identity;

int main()
{
    const auto dir =
        std::filesystem::temp_directory_path() /
        "sovereign_identity_persist_test";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    std::filesystem::create_directories(dir, ec);
    REQUIRE(!ec);

    SessionId session_id{};
    PrincipalId owner_id{};

    // Process 1: bootstrap + authenticate, then close (simulates shutdown)
    {
        FileIdentityStore store;
        REQUIRE(store.Open(dir) == IdentityError::Ok);
        IdentityConfig config = DefaultIdentityConfig();
        config.session_lifetime_seconds = 7200;
        config.data_directory = dir;
        IdentityService identity(store, config);

        auto boot = identity.BootstrapOwner("owner", "secret");
        REQUIRE(boot.Ok());
        owner_id = boot.value;

        auto session = identity.Authenticate("owner", "secret");
        REQUIRE(session.Ok());
        session_id = session.value.id;
        store.Close();
    }

    // Process 2: reopen same directory — sessions must survive (IQ-1A)
    {
        FileIdentityStore store;
        REQUIRE(store.Open(dir) == IdentityError::Ok);
        IdentityConfig config = DefaultIdentityConfig();
        config.session_lifetime_seconds = 7200;
        config.data_directory = dir;
        IdentityService identity(store, config);

        auto valid = identity.ValidateSession(session_id);
        REQUIRE(valid.Ok());
        REQUIRE(valid.value.principal_id == owner_id);

        auto owner = identity.GetPrincipal(owner_id);
        REQUIRE(owner.Ok());
        REQUIRE(owner.value.display_name == "owner");

        // Still cannot double-bootstrap
        auto again = identity.BootstrapOwner("other", "x");
        REQUIRE(again.error == IdentityError::AlreadyBootstrapped);

        store.Close();
    }

    std::filesystem::remove_all(dir, ec);
    std::cout << "identity_persistence_smoke_test: OK\n";
    return 0;
}
