#include "workspace_service.hpp"
#include "test_require.hpp"
#include <filesystem>
#include <iostream>

using namespace sovereign::workspace;

int main()
{
    const auto dir =
        std::filesystem::temp_directory_path() / "sov_ws_test";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    WorkspaceId id{};
    {
        WorkspaceService ws;
        REQUIRE(ws.Open(dir) == WorkspaceError::Ok);
        auto c = ws.Create("dev", "development");
        REQUIRE(c.Ok());
        id = c.value;
        REQUIRE(ws.List().value.size() == 1);
        REQUIRE(ws.Archive(id).Ok());
        REQUIRE(ws.Get(id).value.status == WorkspaceStatus::Archived);
        ws.Close();
    }
    {
        WorkspaceService ws;
        REQUIRE(ws.Open(dir) == WorkspaceError::Ok);
        REQUIRE(ws.Get(id).value.name == "dev");
        ws.Close();
    }
    std::filesystem::remove_all(dir, ec);
    std::cout << "workspace_service_smoke_test: OK\n";
    return 0;
}
