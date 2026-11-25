#include "action_executor.hpp"

#include <chrono>

namespace sovereign::executor
{

ActionExecutor::ActionExecutor(
    sovereign::cognitive::CognitiveService& cognitive,
    const sovereign::policy::PolicyEngine* policy)
    : cognitive_(cognitive)
    , policy_(policy)
{
}

void ActionExecutor::RegisterHandler(const std::string& action, ActionHandler handler)
{
    handlers_.emplace_back(action, std::move(handler));
}

Result<ExecutionRecord> ActionExecutor::Execute(
    const std::string& action,
    const std::array<std::uint8_t, 16>& governance_decision_id,
    const std::vector<std::string>& actor_roles)
{
    Result<ExecutionRecord> result;
    if (action.empty())
    {
        result.error = ExecutorError::InvalidArgument;
        return result;
    }

    auto gate = cognitive_.AssertApproved(governance_decision_id);
    if (!gate.Ok())
    {
        result.error = ExecutorError::NotApproved;
        return result;
    }

    if (policy_ != nullptr && !actor_roles.empty())
    {
        sovereign::PrincipalId dummy{};
        auto pol = policy_->Evaluate(dummy, actor_roles, action);
        if (!pol.Ok() || pol.value.effect != sovereign::policy::PolicyEffect::Allow)
        {
            result.error = ExecutorError::PolicyDenied;
            return result;
        }
    }

    ActionHandler* handler = nullptr;
    for (auto& [name, fn] : handlers_)
    {
        if (name == action)
        {
            handler = &fn;
            break;
        }
    }
    if (handler == nullptr)
    {
        result.error = ExecutorError::UnknownAction;
        return result;
    }

    auto outcome = (*handler)(action);
    ExecutionRecord rec;
    rec.action = action;
    rec.governance_decision_id = governance_decision_id;
    rec.executed_unix_seconds = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
    if (!outcome.Ok())
    {
        rec.succeeded = false;
        rec.detail = "handler failed";
        history_.push_back(rec);
        result.error = ExecutorError::HandlerFailed;
        result.value = rec;
        return result;
    }
    rec.succeeded = true;
    rec.detail = outcome.value;
    history_.push_back(rec);
    result.value = rec;
    return result;
}

VerificationReceipt ActionExecutor::VerifyOutcome(
    const std::string& action,
    const std::string& check_kind,
    const std::string& expected_token,
    const std::string& observed_output)
{
    VerificationReceipt rec;
    rec.action = action;
    rec.check_kind = check_kind.empty() ? "non_empty_output" : check_kind;
    std::uint64_t h = 14695981039346656037ULL;
    for (unsigned char c : observed_output) { h ^= c; h *= 1099511628211ULL; }
    std::string hex(16, '0');
    std::snprintf(hex.data(), hex.size() + 1, "%016llx", static_cast<unsigned long long>(h));
    rec.evidence_hash = hex;
    if (observed_output.empty())
    {
        rec.verified = false;
        rec.observed_detail = "empty_observed_output";
        return rec;
    }
    if (!expected_token.empty() && observed_output.find(expected_token) == std::string::npos)
    {
        rec.verified = false;
        rec.observed_detail = "missing_expected_token:" + expected_token;
        return rec;
    }
    rec.verified = true;
    rec.observed_detail = observed_output;
    return rec;
}

} // namespace sovereign::executor
