#pragma once

#include "runtime_service_capability_pipeline_diagnostics.hpp"

#include "runtime_service_capability_negotiator.hpp"
#include "runtime_service_invocation.hpp"
#include "runtime_service_invoker.hpp"
#include "runtime_service_capability_trace_query.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>

namespace sovereign
{

class RuntimeServiceCapabilityInvocationPolicy;

struct
RuntimeServiceCapabilityInvocationPolicyDecision;

enum class RuntimeServiceCapabilityInvocationStatus :
    std::uint8_t
{
    NoAttempt = 0U,
    NegotiationRejected,
    InvocationRejected,
    Completed,
    CapabilityInvokerUnavailable,
    PolicyRejected
};

struct RuntimeServiceCapabilityInvocationRequest final
{
    std::string capability{};
    std::string preferred_service_name{};
    std::string operation{};
    std::string payload{};
};

struct RuntimeServiceCapabilityInvocationResult final
{
    RuntimeServiceCapabilityInvocationStatus status{
        RuntimeServiceCapabilityInvocationStatus::
            NoAttempt};

    RuntimeServiceCapabilityResult negotiation{};
    RuntimeServiceInvocationResult invocation{};

    std::string policy_capability{};
    std::string policy_operation{};
    std::string policy_provider_service_name{};

    std::uint8_t policy_status{0U};

    std::uint64_t capability_invocation_sequence{0U};

    std::string diagnostic{};

    [[nodiscard]]
    bool Completed() const noexcept
    {
        return
            status ==
                RuntimeServiceCapabilityInvocationStatus::
                    Completed &&
            negotiation.Accepted() &&
            invocation.Completed();
    }
};

class RuntimeServiceCapabilityInvoker final
{
public:
    static constexpr std::size_t
        DefaultTraceCapacity{1024U};

    RuntimeServiceCapabilityInvoker(
        RuntimeServiceCapabilityNegotiator&
            negotiator,
        RuntimeServiceInvoker& invoker) noexcept;

    RuntimeServiceCapabilityInvoker(
        RuntimeServiceCapabilityNegotiator&
            negotiator,
        RuntimeServiceInvoker& invoker,
        RuntimeServiceCapabilityInvocationPolicy&
            policy) noexcept;

    [[nodiscard]]
    RuntimeServiceCapabilityInvocationResult
    Invoke(
        RuntimeServiceCapabilityInvocationRequest
            request);

    [[nodiscard]]
    const RuntimeServiceCapabilityInvocationResult&
    LastInvocation() const noexcept;

    [[nodiscard]]
    const RuntimeServiceCapabilityPipelineTrace&
    LastTrace() const noexcept;

    [[nodiscard]]
    const std::deque<
        RuntimeServiceCapabilityPipelineTrace>&
    Traces() const noexcept;

    [[nodiscard]]
    std::size_t TraceCount() const noexcept;

    void SetTraceCapacity(
        std::size_t capacity) noexcept;

    [[nodiscard]]
    std::size_t TraceCapacity() const noexcept;

    [[nodiscard]]
    std::uint64_t DiscardedTraceCount()
        const noexcept;

    void ClearTraces() noexcept;

    [[nodiscard]]
    std::uint64_t InvocationCount() const noexcept;

    [[nodiscard]]
    std::uint64_t CompletedCount() const noexcept;

    [[nodiscard]]
    std::uint64_t PolicyRejectedCount()
        const noexcept;

    [[nodiscard]]
    RuntimeServiceCapabilityTraceQueryResult
    QueryTraces(
        const RuntimeServiceCapabilityTraceQuery&
            query) const;

    [[nodiscard]]
    std::optional<
        RuntimeServiceCapabilityPipelineTrace>
    FindTrace(
        const Uuid& trace_id) const;


private:
    RuntimeServiceCapabilityNegotiator& negotiator_;
    RuntimeServiceInvoker& invoker_;

    RuntimeServiceCapabilityInvocationPolicy*
        policy_{nullptr};

    RuntimeServiceCapabilityInvocationResult
        last_invocation_{};

    RuntimeServiceCapabilityPipelineTrace
        last_trace_{};

    std::deque<
        RuntimeServiceCapabilityPipelineTrace>
            traces_{};

    std::size_t trace_capacity_{
        DefaultTraceCapacity};

    std::uint64_t discarded_trace_count_{0U};
    std::uint64_t invocation_count_{0U};
    std::uint64_t completed_count_{0U};
    std::uint64_t policy_rejected_count_{0U};
};

} // namespace sovereign
