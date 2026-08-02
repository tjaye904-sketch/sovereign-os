#include "memory_engine.hpp"
#include "test_require.hpp"
#include <filesystem>
#include <iostream>

using namespace sovereign::memory;

int main()
{
    const auto dir =
        std::filesystem::temp_directory_path() / "sov_memory_test";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);

    RecordId id{};
    {
        MemoryEngine mem;
        REQUIRE(mem.Open(dir) == MemoryError::Ok);
        MemoryRecord r;
        r.tags = {"project", "sovereign"};
        r.body = "Identity validated with Argon2 debt";
        auto app = mem.Append(r);
        REQUIRE(app.Ok());
        id = app.value;
        auto got = mem.Get(id);
        REQUIRE(got.Ok());
        REQUIRE(got.value.body.find("Identity") != std::string::npos);
        auto by = mem.ListByTag("sovereign");
        REQUIRE(by.Ok());
        REQUIRE(by.value.size() == 1);
        mem.Close();
    }
    {
        MemoryEngine mem;
        REQUIRE(mem.Open(dir) == MemoryError::Ok);
        auto got = mem.Get(id);
        REQUIRE(got.Ok());
        REQUIRE(got.value.tags.size() >= 1);
        mem.Close();
    }
    std::filesystem::remove_all(dir, ec);
    std::cout << "memory_engine_smoke_test: OK\n";
    return 0;
}
