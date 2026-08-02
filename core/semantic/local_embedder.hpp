#pragma once

#include "semantic_types.hpp"

#include <string>

namespace sovereign::semantic
{

// Hashing-trick fallback (always available offline).
Embedding EmbedTextHash(const std::string& text);

// Prefer local Ollama embedding model when reachable.
// Env: OLLAMA_HOST, SOVEREIGN_EMBED_MODEL (default nomic-embed-text)
Embedding EmbedText(const std::string& text, std::string* model_used = nullptr);

float CosineSimilarity(const Embedding& a, const Embedding& b);

} // namespace sovereign::semantic
