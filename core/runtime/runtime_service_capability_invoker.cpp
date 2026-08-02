#include "runtime_service_capability_invoker.hpp"

#include <chrono>
#include "runtime_service_capability_invocation_policy.hpp"

#include <utility>

namespace sovereign
{

RuntimeServiceCapabilityInvoker::
RuntimeServiceCapabilityInvoker(
    RuntimeServiceCapabilityNegotiator& negotiator,
    RuntimeServiceInvoker& invoker) noexcept
    :
    negotiator_(negotiator),
    invoker_(invoker)
{
}

RuntimeServiceCapabilityInvoker::
RuntimeServiceCapabilityInvoker(
    RuntimeServiceCapabilityNegotiator& negotiator,
    RuntimeServiceInvoker& invoker,
    RuntimeServiceCapabilityInvocationPolicy&
        policy) noexcept
    :
    negotiator_(negotiator),
    invoker_(invoker),
    policy_(&policy)
{
}

RuntimeServiceCapabilityInvocationResult
RuntimeServiceCapabilityInvoker::Invoke(
    RuntimeServiceCapabilityInvocationRequest request)
{
    using Clock = std::chrono::steady_clock;

    const auto now_nanoseconds =
        []() -> std::uint64_t
        {
            const auto duration =
                Clock::now().time_since_epoch();

            const auto nanoseconds =
                std::chrono::duration_cast<
                    std::chrono::nanoseconds>(
                        duration);

            return static_cast<std::uint64_t>(
                nanoseconds.count());
        };

    RuntimeServiceCapabilityPipelineTrace trace{};

    trace.trace_id = Uuid::Generate();

    trace.started_nanoseconds =
        now_nanoseconds();

    trace.capability = request.capability;

    trace.preferred_service_name =
        request.preferred_service_name;

    trace.operation = request.operation;

    RuntimeServiceCapabilityInvocationResult result{};

    result.capability_invocation_sequence =
        ++invocation_count_;

    trace.invocation_sequence =
        result.capability_invocation_sequence;

    const auto complete_trace =
        [this, &trace, &result, &now_nanoseconds]()
        {
            trace.result_status =
                static_cast<std::uint8_t>(
                    result.status);

            trace.completed =
                result.Completed();

            trace.diagnostic =
                result.diagnostic;

            trace.ended_nanoseconds =
                now_nanoseconds();

            trace.total_duration_nanoseconds =
                trace.ended_nanoseconds -
                trace.started_nanoseconds;

            if (trace_capacity_ == 0U)
            {
                ++discarded_trace_count_;
                last_trace_ = {};
            }
            else
            {
                if (traces_.size() ==
                    trace_capacity_)
                {
                    traces_.pop_front();
                    ++discarded_trace_count_;
                }

                traces_.push_back(trace);
                last_trace_ = traces_.back();
            }

            last_invocation_ = result;
        };

    if (policy_ != nullptr)
    {
        trace.request_policy_evaluated = true;

        trace.request_policy_started_nanoseconds =
            now_nanoseconds();

        const auto decision =
            policy_->EvaluateRequest(request);

        trace.request_policy_ended_nanoseconds =
            now_nanoseconds();

        trace.request_policy_duration_nanoseconds =
            trace.request_policy_ended_nanoseconds -
            trace.request_policy_started_nanoseconds;

        trace.request_policy_status =
            static_cast<std::uint8_t>(
                decision.status);

        result.policy_status =
            static_cast<std::uint8_t>(
                decision.status);

        result.policy_capability =
            decision.capability;

        result.policy_operation =
            decision.operation;

        result.policy_provider_service_name =
            decision.provider_service_name;

        if (!decision.Allowed())
        {
            result.status =
                RuntimeServiceCapabilityInvocationStatus::
                    PolicyRejected;

            result.diagnostic =
                decision.diagnostic;

            ++policy_rejected_count_;

            complete_trace();
            return result;
        }
    }

    trace.negotiation_attempted = true;

    trace.negotiation_started_nanoseconds =
        now_nanoseconds();

    result.negotiation =
        negotiator_.Negotiate(
            RuntimeServiceCapabilityRequest{
                .capability =
                    request.capability,
                .preferred_service_name =
                    request.preferred_service_name,
                .require_running = true});

    trace.negotiation_ended_nanoseconds =
        now_nanoseconds();

    trace.negotiation_duration_nanoseconds =
        trace.negotiation_ended_nanoseconds -
        trace.negotiation_started_nanoseconds;

    trace.negotiation_status =
        static_cast<std::uint8_t>(
            result.negotiation.status);

    trace.candidate_count =
        result.negotiation.candidate_count;

    trace.selected_service_name =
        result.negotiation.selected_service_name;

    if (!result.negotiation.Accepted())
    {
        result.status =
            RuntimeServiceCapabilityInvocationStatus::
                NegotiationRejected;

        result.diagnostic =
            result.negotiation.diagnostic;

        complete_trace();
        return result;
    }

    if (policy_ != nullptr)
    {
        trace.provider_policy_evaluated = true;

        trace.provider_policy_started_nanoseconds =
            now_nanoseconds();

        const auto decision =
            policy_->EvaluateProvider(
                request,
                result.negotiation
                    .selected_service_name);

        trace.provider_policy_ended_nanoseconds =
            now_nanoseconds();

        trace.provider_policy_duration_nanoseconds =
            trace.provider_policy_ended_nanoseconds -
            trace.provider_policy_started_nanoseconds;

        trace.provider_policy_status =
            static_cast<std::uint8_t>(
                decision.status);

        result.policy_status =
            static_cast<std::uint8_t>(
                decision.status);

        result.policy_capability =
            decision.capability;

        result.policy_operation =
            decision.operation;

        result.policy_provider_service_name =
            decision.provider_service_name;

        if (!decision.Allowed())
        {
            result.status =
                RuntimeServiceCapabilityInvocationStatus::
                    PolicyRejected;

            result.diagnostic =
                decision.diagnostic;

            ++policy_rejected_count_;

            complete_trace();
            return result;
        }
    }

    trace.invocation_attempted = true;

    trace.invocation_started_nanoseconds =
        now_nanoseconds();

    result.invocation =
        invoker_.Invoke(
            RuntimeServiceInvocationRequest{
                .service_name =
                    result.negotiation
                        .selected_service_name,
                .operation =
                    std::move(request.operation),
                .payload =
                    std::move(request.payload)});

    trace.invocation_ended_nanoseconds =
        now_nanoseconds();

    trace.invocation_duration_nanoseconds =
        trace.invocation_ended_nanoseconds -
        trace.invocation_started_nanoseconds;

    trace.invocation_status =
        static_cast<std::uint8_t>(
            result.invocation.status);

    if (!result.invocation.Completed())
    {
        result.status =
            RuntimeServiceCapabilityInvocationStatus::
                InvocationRejected;

        result.diagnostic =
            result.invocation.diagnostic;

        complete_trace();
        return result;
    }

    result.status =
        RuntimeServiceCapabilityInvocationStatus::
            Completed;

    ++completed_count_;

    complete_trace();
    return result;
}

const RuntimeServiceCapabilityInvocationResult&
RuntimeServiceCapabilityInvoker::
LastInvocation() const noexcept
{
    return last_invocation_;
}

const RuntimeServiceCapabilityPipelineTrace&
RuntimeServiceCapabilityInvoker::
LastTrace() const noexcept
{
    return last_trace_;
}

const std::deque<
    RuntimeServiceCapabilityPipelineTrace>&
RuntimeServiceCapabilityInvoker::
Traces() const noexcept
{
    return traces_;
}

std::size_t
RuntimeServiceCapabilityInvoker::
TraceCount() const noexcept
{
    return traces_.size();
}

void
RuntimeServiceCapabilityInvoker::
SetTraceCapacity(
    const std::size_t capacity) noexcept
{
    trace_capacity_ = capacity;

    while (traces_.size() > trace_capacity_)
    {
        traces_.pop_front();
        ++discarded_trace_count_;
    }

    if (traces_.empty())
    {
        last_trace_ = {};
        return;
    }

    last_trace_ = traces_.back();
}

std::size_t
RuntimeServiceCapabilityInvoker::
TraceCapacity() const noexcept
{
    return trace_capacity_;
}

std::uint64_t
RuntimeServiceCapabilityInvoker::
DiscardedTraceCount() const noexcept
{
    return discarded_trace_count_;
}

void
RuntimeServiceCapabilityInvoker::
ClearTraces() noexcept
{
    traces_.clear();
    last_trace_ = {};
}

std::uint64_t
RuntimeServiceCapabilityInvoker::
InvocationCount() const noexcept
{
    return invocation_count_;
}

std::uint64_t
RuntimeServiceCapabilityInvoker::
CompletedCount() const noexcept
{
    return completed_count_;
}

std::uint64_t
RuntimeServiceCapabilityInvoker::
PolicyRejectedCount() const noexcept
{
    return policy_rejected_count_;
}

} // namespace sovereign

namespace sovereign
{

RuntimeServiceCapabilityTraceQueryResult
RuntimeServiceCapabilityInvoker::QueryTraces(
    const RuntimeServiceCapabilityTraceQuery&
        query) const
{
    return QueryRuntimeServiceCapabilityTraces(
        traces_,
        query);
}

std::optional<
    RuntimeServiceCapabilityPipelineTrace>
RuntimeServiceCapabilityInvoker::FindTrace(
    const Uuid& trace_id) const
{
    return FindRuntimeServiceCapabilityTrace(
        traces_,
        trace_id);
}

} // namespace sovereign
