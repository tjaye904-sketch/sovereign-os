#include "workspace_service.hpp"

#include <filesystem>
#include <iostream>

#define REQUIRE(cond) \
    do { if (!(cond)) { std::cerr << "REQUIRE failed: " #cond "\n"; return 1; } } while (0)

int main()
{
    namespace fs = std::filesystem;
    auto dir = fs::temp_directory_path() / "sov-ws-ctrl";
    fs::remove_all(dir);
    fs::create_directories(dir);

    sovereign::workspace::WorkspaceService ws;
    REQUIRE(ws.Open(dir) == sovereign::workspace::WorkspaceError::Ok);

    auto id = ws.Create("dev-main", "development");
    REQUIRE(id.Ok());

    auto act = ws.Get(id.value);
    REQUIRE(act.Ok());
    REQUIRE(act.value.status == sovereign::workspace::WorkspaceStatus::Active);

    REQUIRE(ws.Suspend(id.value).Ok());
    auto sus = ws.Get(id.value);
    REQUIRE(sus.Ok());
    REQUIRE(sus.value.status == sovereign::workspace::WorkspaceStatus::Suspended);
    REQUIRE(std::string(sovereign::workspace::WorkspaceService::StatusName(sus.value.status)) == "suspended");

    REQUIRE(ws.Activate(id.value).Ok());
    auto again = ws.Get(id.value);
    REQUIRE(again.value.status == sovereign::workspace::WorkspaceStatus::Active);

    auto by = ws.FindByName("dev-main");
    REQUIRE(by.Ok());
    REQUIRE(by.value.kind == "development");

    auto only_sus = ws.ListByStatus(sovereign::workspace::WorkspaceStatus::Suspended);
    REQUIRE(only_sus.Ok());
    REQUIRE(only_sus.value.empty());

    REQUIRE(ws.Suspend(id.value).Ok());
    only_sus = ws.ListByStatus(sovereign::workspace::WorkspaceStatus::Suspended);
    REQUIRE(only_sus.value.size() == 1);

    ws.Close();
    fs::remove_all(dir);
    std::cout << "workspace_controller_smoke_test: OK\n";
    return 0;
}
