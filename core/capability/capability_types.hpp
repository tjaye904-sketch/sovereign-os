#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::capability
{

enum class CapabilityError : std::uint8_t
{
    Ok = 0,
    NotFound,
    InvalidArgument,
    StorageFailure,
    NotOpen,
};

template <typename T>
struct Result
{
    CapabilityError error{CapabilityError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept { return error == CapabilityError::Ok; }
};

template <>
struct Result<void>
{
    CapabilityError error{CapabilityError::Ok};
    [[nodiscard]] bool Ok() const noexcept { return error == CapabilityError::Ok; }
    static Result OkRes() { return Result{CapabilityError::Ok}; }
    static Result Err(CapabilityError e) { return Result{e}; }
};

struct Capability
{
    std::string id;          // e.g. host.computer_use, model.chat
    std::string description;
    bool enabled{true};
    std::string source;      // seed | scan | operator
};

} // namespace sovereign::capability
