#include "runtime_service_invoker.hpp"

#include <exception>
#include <utility>

namespace sovereign
{

RuntimeServiceInvoker::RuntimeServiceInvoker(
    ServiceRegistry& registry) noexcept
    :
    registry_(registry)
{
}

RuntimeServiceInvocationResult
RuntimeServiceInvoker::Invoke(
    RuntimeServiceInvocationRequest request)
{
    RuntimeServiceInvocationResult result{};

    result.service_name =
        std::move(request.service_name);

    result.operation =
        std::move(request.operation);

    result.invocation_sequence =
        ++invocation_count_;

    if (result.service_name.empty())
    {
        result.status =
            RuntimeServiceInvocationStatus::
                EmptyServiceName;

        result.diagnostic =
            "Runtime service invocation requires "
            "a service name.";

        last_invocation_ = result;
        return result;
    }

    if (result.operation.empty())
    {
        result.status =
            RuntimeServiceInvocationStatus::
                EmptyOperation;

        result.diagnostic =
            "Runtime service invocation requires "
            "an operation.";

        last_invocation_ = result;
        return result;
    }

    Service* service =
        registry_.Find(result.service_name);

    if (service == nullptr)
    {
        result.status =
            RuntimeServiceInvocationStatus::
                ServiceNotFound;

        result.diagnostic =
            "The requested runtime service is "
            "not registered.";

        last_invocation_ = result;
        return result;
    }

    if (service->State() != ServiceState::Running)
    {
        result.status =
            RuntimeServiceInvocationStatus::
                ServiceNotRunning;

        result.diagnostic =
            "The requested runtime service is "
            "not running.";

        last_invocation_ = result;
        return result;
    }

    auto* invocable =
        dynamic_cast<RuntimeInvocableService*>(
            service);

    if (invocable == nullptr)
    {
        result.status =
            RuntimeServiceInvocationStatus::
                ServiceNotInvocable;

        result.diagnostic =
            "The requested runtime service does "
            "not implement invocation.";

        last_invocation_ = result;
        return result;
    }

    if (!invocable->SupportsOperation(
            result.operation))
    {
        result.status =
            RuntimeServiceInvocationStatus::
                OperationRejected;

        result.diagnostic =
            "The runtime service does not support "
            "the requested operation.";

        last_invocation_ = result;
        return result;
    }

    try
    {
        std::string response{};

        if (!invocable->Invoke(
                result.operation,
                request.payload,
                response))
        {
            result.status =
                RuntimeServiceInvocationStatus::
                    OperationRejected;

            result.diagnostic =
                "The runtime service rejected "
                "the invocation.";

            last_invocation_ = result;
            return result;
        }

        result.status =
            RuntimeServiceInvocationStatus::
                Completed;

        result.response_payload =
            std::move(response);

        ++completed_count_;
    }
    catch (const std::exception& exception)
    {
        result.status =
            RuntimeServiceInvocationStatus::
                HandlerException;

        result.diagnostic = exception.what();
    }
    catch (...)
    {
        result.status =
            RuntimeServiceInvocationStatus::
                HandlerException;

        result.diagnostic =
            "The runtime service invocation "
            "raised an unknown exception.";
    }

    last_invocation_ = result;
    return result;
}

const RuntimeServiceInvocationResult&
RuntimeServiceInvoker::LastInvocation() const noexcept
{
    return last_invocation_;
}

std::uint64_t
RuntimeServiceInvoker::InvocationCount() const noexcept
{
    return invocation_count_;
}

std::uint64_t
RuntimeServiceInvoker::CompletedCount() const noexcept
{
    return completed_count_;
}

} // namespace sovereign
