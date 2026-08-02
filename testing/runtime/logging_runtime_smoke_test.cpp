#include "log_manager.hpp"
#include "memory_log_sink.hpp"

#include <memory>
#include <string>
#include "test_require.hpp"

int main()
{
    sovereign::LogManager manager;

    auto sink =
        std::make_shared<sovereign::MemoryLogSink>();

    REQUIRE(manager.RegisterSink(sink));
    REQUIRE(!manager.RegisterSink(nullptr));
    REQUIRE(manager.SinkCount() == 1U);

    sovereign::LogRecord first;
    first.level = sovereign::LogLevel::Information;
    first.category = "Runtime";
    first.message = "Sovereign runtime started.";

    sovereign::LogRecord second;
    second.level = sovereign::LogLevel::Warning;
    second.category = "Scheduler";
    second.message = "Frame budget exceeded.";

    REQUIRE(manager.Write(first));
    REQUIRE(manager.Write(second));

    REQUIRE(sink->Size() == 2U);

    const auto records = sink->Records();

    REQUIRE(records.size() == 2U);
    REQUIRE(records[0].category == "Runtime");
    REQUIRE(records[0].message ==
           "Sovereign runtime started.");
    REQUIRE(records[1].level ==
           sovereign::LogLevel::Warning);
    REQUIRE(records[1].category == "Scheduler");

    sink->Clear();

    REQUIRE(sink->Size() == 0U);

    return 0;
}
