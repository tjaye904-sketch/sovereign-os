#pragma once

#include "service.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace sovereign
{

struct RuntimeServiceInvocationRequest final
{
    std::string service_name{};
    std::string operation{};
    std::string payload{};
};

enum class RuntimeServiceInvocationStatus
{
    NoAttempt,
    Completed,
    EmptyServiceName,
    EmptyOperation,
    ServiceNotFound,
    ServiceNotRunning,
    ServiceNotInvocable,
    OperationRejected,
    HandlerException
};

struct RuntimeServiceInvocationResult final
{
    RuntimeServiceInvocationStatus status{
        RuntimeServiceInvocationStatus::NoAttempt};

    std::string service_name{};
    std::string operation{};
    std::string response_payload{};
    std::string diagnostic{};

    std::uint64_t invocation_sequence{0U};

    [[nodiscard]]
    bool Completed() const noexcept
    {
        return
            status ==
            RuntimeServiceInvocationStatus::Completed;
    }
};

class RuntimeInvocableService
{
public:
    virtual ~RuntimeInvocableService() = default;

    [[nodiscard]]
    virtual bool SupportsOperation(
        std::string_view operation) const noexcept = 0;

    [[nodiscard]]
    virtual bool Invoke(
        std::string_view operation,
        std::string_view request_payload,
        std::string& response_payload) = 0;
};

} // namespace sovereign
