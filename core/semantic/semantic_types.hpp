#pragma once

#include "principal_id.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace sovereign::semantic
{

using ::sovereign::PrincipalId;

// Variable-length embedding vector (local model or fallback).
struct Embedding
{
    std::vector<float> v;
};

struct SemanticRecordId
{
    std::array<std::uint8_t, 16> bytes{};
};

struct SemanticRecord
{
    SemanticRecordId id{};
    PrincipalId author{};
    std::string text;
    Embedding embedding{};
    std::string embed_model; // e.g. nomic-embed-text or "hash-v1"
    std::vector<std::string> tags;
    std::uint64_t created_unix_seconds{0};
};

struct ScoredRecord
{
    SemanticRecord record;
    float score{0.0f};
};

enum class SemanticError : std::uint8_t
{
    Ok = 0,
    InvalidArgument,
    StorageFailure,
    NotFound,
    EmbedFailure
};

template <typename T>
struct Result
{
    SemanticError error{SemanticError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == SemanticError::Ok;
    }
};

} // namespace sovereign::semantic
