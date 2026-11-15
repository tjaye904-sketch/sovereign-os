#include "test_require.hpp"

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>

namespace
{
std::string RunCapture(const std::string& command)
{
    const std::string full_cmd = command + " 2>&1";
    FILE* pipe = popen(full_cmd.c_str(), "r");
    REQUIRE(pipe != nullptr);

    std::string output;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        output += buffer;
    }

    const int rc = pclose(pipe);
    REQUIRE(rc == 0);
    return output;
}

std::pair<int, std::string> RunCaptureAllowFailure(const std::string& command)
{
    const std::string full_cmd = command + " 2>&1";
    FILE* pipe = popen(full_cmd.c_str(), "r");
    REQUIRE(pipe != nullptr);

    std::string output;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        output += buffer;
    }

    const int rc = pclose(pipe);
    return {rc, output};
}

std::string ExtractField(const std::string& text, const std::string& key)
{
    const auto pos = text.find(key);
    REQUIRE(pos != std::string::npos);
    const auto value = text.substr(pos + key.size());
    const auto newline = value.find('\n');
    if (newline == std::string::npos)
        return value;
    return value.substr(0, newline);
}
} // namespace

int main()
{
    const std::string cli = "/home/tjaye904/Documents/GitHub/sovereign-os/build/apps/cli/sovereign-cli";
    const auto root = std::filesystem::temp_directory_path() / "sov_cli_role_management_test";
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    std::filesystem::create_directories(root, ec);

    const std::string data_env = "SOVEREIGN_DATA_ROOT=" + root.string();

    const std::string boot_cmd = data_env + " " + cli + " bootstrap owner secret-pass";
    const auto boot = RunCapture(boot_cmd);
    const std::string owner_id = ExtractField(boot, "owner_id=");

    const std::string login_cmd = data_env + " " + cli + " login owner secret-pass";
    static_cast<void>(RunCapture(login_cmd));

    const std::string add_role_cmd = data_env + " " + cli + " role-add " + owner_id + " admin";
    static_cast<void>(RunCapture(add_role_cmd));

    const std::string list_roles_cmd = data_env + " " + cli + " roles " + owner_id;
    const auto roles = RunCapture(list_roles_cmd);
    REQUIRE(roles.find("admin") != std::string::npos);

    const std::string missing_principal = "00000000000000000000000000000000";
    const std::string missing_role_cmd = data_env + " " + cli + " role-add " + missing_principal + " admin";
    const auto missing_role = RunCaptureAllowFailure(missing_role_cmd);
    REQUIRE(missing_role.first != 0);
    REQUIRE(missing_role.second.find("principal_not_found") != std::string::npos ||
            missing_role.second.find("not_found") != std::string::npos);

    const std::string alice_name = "alice";
    const std::string create_operator_cmd = data_env + " " + cli + " operator-create " + alice_name + " alice-secret";
    const auto create_operator = RunCapture(create_operator_cmd);
    const std::string alice_id = ExtractField(create_operator, "operator_id=");

    const std::string login_alice_cmd = data_env + " " + cli + " login " + alice_name + " alice-secret";
    static_cast<void>(RunCapture(login_alice_cmd));

    const std::string unauthorized_create_cmd = data_env + " " + cli + " operator-create bob bob-secret";
    const auto unauthorized_create = RunCaptureAllowFailure(unauthorized_create_cmd);
    REQUIRE(unauthorized_create.first != 0);
    REQUIRE(unauthorized_create.second.find("unauthorized") != std::string::npos);

    std::filesystem::remove_all(root, ec);
    std::cout << "cli_role_management_smoke_test: OK\n";
    return 0;
}
