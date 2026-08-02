#include "sync_service.hpp"
#include "test_require.hpp"
#include <filesystem>
#include <iostream>
using namespace sovereign::sync;
int main()
{
    const auto dir = std::filesystem::temp_directory_path() / "sov_sync_test";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    PrincipalId actor{};
    actor.bytes[0] = 5;
    {
        SyncService sync;
        REQUIRE(sync.Open(dir) == SyncError::Ok);
        REQUIRE(sync.RecordExport(actor, "folder", "/tmp/bundle").Ok());
        REQUIRE(sync.List().value.size() == 1);
        sync.Close();
    }
    {
        SyncService sync;
        REQUIRE(sync.Open(dir) == SyncError::Ok);
        REQUIRE(sync.List().value[0].channel == "folder");
        sync.Close();
    }
    std::filesystem::remove_all(dir, ec);
    std::cout << "sync_service_smoke_test: OK\n";
    return 0;
}
