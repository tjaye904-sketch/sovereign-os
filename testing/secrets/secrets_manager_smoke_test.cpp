#include "secret_manager.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#define REQUIRE(cond)                                                                          \
    do                                                                                         \
    {                                                                                          \
        if (!(cond))                                                                           \
        {                                                                                      \
            std::cerr << "REQUIRE failed: " #cond " at " << __FILE__ << ":" << __LINE__ << "\n"; \
            return 1;                                                                          \
        }                                                                                      \
    } while (0)

int main()
{
    namespace fs = std::filesystem;
    auto root = fs::temp_directory_path() / ("sov-secrets-" + std::to_string(std::rand()));
    fs::create_directories(root);

    int audits = 0;
    sovereign::secrets::SecretManager sm;
    REQUIRE(sm.Open(root) == sovereign::secrets::SecretError::Ok);
    sm.SetAuditHook([&](const std::string&, const std::string&) { ++audits; });

    auto put = sm.Put("ollama_token", "super-secret-value");
    REQUIRE(put.Ok());
    auto ex = sm.Exists("ollama_token");
    REQUIRE(ex.Ok() && ex.value);
    auto got = sm.Get("ollama_token");
    REQUIRE(got.Ok());
    REQUIRE(got.value == "super-secret-value");

    auto bad = sm.Put("../escape", "x");
    REQUIRE(!bad.Ok());

    auto list = sm.List();
    REQUIRE(list.Ok());
    REQUIRE(list.value.size() == 1);
    REQUIRE(list.value[0].name == "ollama_token");
    REQUIRE(list.value[0].value_bytes == std::string("super-secret-value").size());

    auto del = sm.Delete("ollama_token");
    REQUIRE(del.Ok());
    auto ex2 = sm.Exists("ollama_token");
    REQUIRE(ex2.Ok() && !ex2.value);
    REQUIRE(audits >= 3);

    sm.Close();
    fs::remove_all(root);
    std::cout << "secrets_manager_smoke_test: OK\n";
    return 0;
}
