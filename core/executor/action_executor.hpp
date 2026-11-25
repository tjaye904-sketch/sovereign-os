#pragma once

// Executes consequential actions only after Governance approval (AssertApproved).

#include "cognitive_service.hpp"
#include "governance_service.hpp"
#include "policy_engine.hpp"
#include <cstdio>

#include <functional>
#include <string>
#include <vector>

namespace sovereign::executor
{

enum class ExecutorError : std::uint8_t
{
    Ok = 0,
    NotApproved,
    InvalidArgument,
    HandlerFailed,
    UnknownAction,
    PolicyDenied
};

template <typename T>
struct Result
{
    ExecutorError error{ExecutorError::Ok};
    T value{};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == ExecutorError::Ok;
    }
};

template <>
struct Result<void>
{
    ExecutorError error{ExecutorError::Ok};
    [[nodiscard]] bool Ok() const noexcept
    {
        return error == ExecutorError::Ok;
    }
};

struct VerificationReceipt
{
    bool verified{false};
    std::string action;
    std::string check_kind;
    std::string evidence_hash;
    std::string observed_detail;
};

struct ExecutionRecord
{
    std::string action;
    std::array<std::uint8_t, 16> governance_decision_id{};
    bool succeeded{false};
    std::string detail;
    std::uint64_t executed_unix_seconds{0};
};

using ActionHandler = std::function<Result<std::string>(const std::string& action)>;

class ActionExecutor
{
public:
    ActionExecutor(
        sovereign::cognitive::CognitiveService& cognitive,
        const sovereign::policy::PolicyEngine* policy = nullptr);

    void RegisterHandler(const std::string& action, ActionHandler handler);

    // Requires Approved governance decision matching the action proposal path.
    Result<ExecutionRecord> Execute(
        const std::string& action,
        const std::array<std::uint8_t, 16>& governance_decision_id,
        const std::vector<std::string>& actor_roles = {});

    static VerificationReceipt VerifyOutcome(
        const std::string& action,
        const std::string& check_kind,
        const std::string& expected_token,
        const std::string& observed_output);

    const std::vector<ExecutionRecord>& History() const noexcept
    {
        return history_;
    }

private:
    sovereign::cognitive::CognitiveService& cognitive_;
    const sovereign::policy::PolicyEngine* policy_{nullptr};
    std::vector<std::pair<std::string, ActionHandler>> handlers_;
    std::vector<ExecutionRecord> history_;
};

} // namespace sovereign::executor
