#include "config_manager.hpp"

#include <cstdint>
#include <string>
#include <variant>
#include "test_require.hpp"

int main()
{
    sovereign::ConfigManager config;

    REQUIRE(config.Size() == 0U);
    REQUIRE(!config.Contains("runtime.name"));
    REQUIRE(!config.Set("", std::string("invalid")));

    REQUIRE(config.Set(
        "runtime.enabled",
        true));

    REQUIRE(config.Set(
        "runtime.max_frames",
        std::int64_t{120}));

    REQUIRE(config.Set(
        "scheduler.target_hz",
        60.0));

    REQUIRE(config.Set(
        "runtime.name",
        std::string("Sovereign")));

    REQUIRE(config.Size() == 4U);
    REQUIRE(config.Contains("runtime.enabled"));
    REQUIRE(config.Contains("runtime.name"));

    const auto enabled =
        config.Get("runtime.enabled");

    REQUIRE(enabled.has_value());
    REQUIRE(std::get<bool>(*enabled));

    const auto max_frames =
        config.Get("runtime.max_frames");

    REQUIRE(max_frames.has_value());
    REQUIRE(
        std::get<std::int64_t>(*max_frames) ==
        120);

    const auto target_hz =
        config.Get("scheduler.target_hz");

    REQUIRE(target_hz.has_value());
    REQUIRE(
        std::get<double>(*target_hz) ==
        60.0);

    const auto name =
        config.Get("runtime.name");

    REQUIRE(name.has_value());
    REQUIRE(
        std::get<std::string>(*name) ==
        "Sovereign");

    REQUIRE(config.Set(
        "runtime.name",
        std::string("Sovereign Runtime")));

    REQUIRE(config.Size() == 4U);

    const auto updated_name =
        config.Get("runtime.name");

    REQUIRE(updated_name.has_value());
    REQUIRE(
        std::get<std::string>(*updated_name) ==
        "Sovereign Runtime");

    REQUIRE(config.Remove("runtime.enabled"));
    REQUIRE(!config.Remove("runtime.enabled"));
    REQUIRE(!config.Contains("runtime.enabled"));

    REQUIRE(!config.Get("missing.key").has_value());

    config.Clear();

    REQUIRE(config.Size() == 0U);

    return 0;
}
