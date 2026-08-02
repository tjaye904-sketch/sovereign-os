# Model Router — Implementation Specification

**Document ID:** SPEC-MODEL-ROUTER-001  
**Status:** Draft for implementation  
**Date:** 2026-07-29  
**Owner decisions:** Local Ollama only; tool calling enabled.

## Purpose

Route model requests to a local Ollama instance. Replaceable adapter surface
for future providers without changing product services.

## v1 Scope

- HTTP client to `OLLAMA_HOST` (default `http://127.0.0.1:11434`)
- Chat completion with optional tools (Ollama tools JSON)
- List local models (`/api/tags`)
- Health check
- No cloud providers in v1

## Tool calling

- Request may include tool definitions (name, description, parameters JSON schema)
- Response may include tool_calls; caller executes tools and continues turn
- Model Router does not execute tools itself — orchestration layer does

## Governance coupling

Consequential model-driven actions still require Governance approval before
side effects; router only generates text/tool_calls.
