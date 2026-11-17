#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::secrets
{

enum class SecretError : std::uint8_t
{
    Ok = 0,
    InvalidArgument,
    NotFound,
    StorageFailure,
    NotOpen,
};

template <typename T>
struct Result
{
    SecretError error{SecretError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept { return error == SecretError::Ok; }
};

template <>
struct Result<void>
{
    SecretError error{SecretError::Ok};
    [[nodiscard]] bool Ok() const noexcept { return error == SecretError::Ok; }
    static Result OkRes() { return Result{SecretError::Ok}; }
    static Result Err(SecretError e) { return Result{e}; }
};

struct SecretMeta
{
    std::string name;
    std::uint64_t updated_unix{0};
    std::size_t value_bytes{0};
};

} // namespace sovereign::secrets
