#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sovereign
{

enum class RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistrationStatus
{
    registered,
    duplicate_source_version,
    invalid_source_version,
    invalid_target_version,
    identical_versions,
    empty_handler
};

enum class RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathStatus
{
    resolved,
    already_at_target,
    missing_transition,
    cycle_detected,
    hop_limit_exceeded,
    invalid_version
};

enum class RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionStatus
{
    migrated,
    already_at_target,
    path_resolution_failed,
    handler_failed,
    empty_handler_payload,
    version_contract_violation
};

struct RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult
{
    bool succeeded{false};
    std::string payload{};
    std::string diagnostic{};
};

using RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationHandler =
    std::function<
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationStepResult(
            std::string_view)>;

struct RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistration
{
    std::uint32_t source_version{0};
    std::uint32_t target_version{0};

    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationHandler
        handler{};
};

struct RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistrationResult
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistrationStatus
        status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistrationStatus::
                invalid_source_version};

    std::uint32_t source_version{0};
    std::uint32_t target_version{0};
    bool registered{false};
};

struct RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathResult
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathStatus
        status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathStatus::
                invalid_version};

    std::uint32_t source_version{0};
    std::uint32_t target_version{0};

    std::vector<
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistration>
        steps{};

    std::vector<std::uint32_t> visited_versions{};

    bool path_available{false};
};

struct RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionResult
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionStatus
        status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionStatus::
                path_resolution_failed};

    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathResult
        path{};

    std::string migrated_payload{};
    std::string diagnostic{};

    std::uint32_t source_version{0};
    std::uint32_t target_version{0};
    std::size_t completed_steps{0};

    bool migration_succeeded{false};
    bool payload_available{false};
};

class RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistry
{
public:
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistrationResult
    Register(
        const std::uint32_t source_version,
        const std::uint32_t target_version,
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationHandler
            handler)
    {
        using Status =
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistrationStatus;

        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistrationResult
            result;

        result.source_version = source_version;
        result.target_version = target_version;

        if (target_version == 0U)
        {
            result.status = Status::invalid_target_version;
            return result;
        }

        if (source_version == target_version)
        {
            result.status = Status::identical_versions;
            return result;
        }

        if (!handler)
        {
            result.status = Status::empty_handler;
            return result;
        }

        if (registrations_.find(source_version) != registrations_.end())
        {
            result.status = Status::duplicate_source_version;
            return result;
        }

        registrations_.emplace(
            source_version,
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistration{
                source_version,
                target_version,
                std::move(handler)});

        result.status = Status::registered;
        result.registered = true;

        return result;
    }

    [[nodiscard]]
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathResult
    Resolve(
        const std::uint32_t source_version,
        const std::uint32_t target_version,
        const std::size_t maximum_hops = 64U) const
    {
        using Status =
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathStatus;

        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathResult
            result;

        result.source_version = source_version;
        result.target_version = target_version;

        if (target_version == 0U)
        {
            result.status = Status::invalid_version;
            return result;
        }

        result.visited_versions.push_back(source_version);

        if (source_version == target_version)
        {
            result.status = Status::already_at_target;
            result.path_available = true;
            return result;
        }

        std::set<std::uint32_t> visited;
        visited.insert(source_version);

        std::uint32_t current_version = source_version;

        while (current_version != target_version)
        {
            if (result.steps.size() >= maximum_hops)
            {
                result.status = Status::hop_limit_exceeded;
                return result;
            }

            const auto registration =
                registrations_.find(current_version);

            if (registration == registrations_.end())
            {
                result.status = Status::missing_transition;
                return result;
            }

            result.steps.push_back(registration->second);

            current_version =
                registration->second.target_version;

            result.visited_versions.push_back(current_version);

            if (current_version == target_version)
            {
                result.status = Status::resolved;
                result.path_available = true;
                return result;
            }

            if (!visited.insert(current_version).second)
            {
                result.status = Status::cycle_detected;
                return result;
            }
        }

        result.status = Status::resolved;
        result.path_available = true;

        return result;
    }

    [[nodiscard]]
    RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionResult
    Execute(
        const std::uint32_t source_version,
        const std::uint32_t target_version,
        const std::string_view payload,
        const std::size_t maximum_hops = 64U) const
    {
        using ExecutionStatus =
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionStatus;

        using PathStatus =
            RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationPathStatus;

        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationExecutionResult
            result;

        result.source_version = source_version;
        result.target_version = target_version;

        result.path =
            Resolve(
                source_version,
                target_version,
                maximum_hops);

        if (!result.path.path_available)
        {
            result.status = ExecutionStatus::path_resolution_failed;
            return result;
        }

        if (result.path.status == PathStatus::already_at_target)
        {
            result.status = ExecutionStatus::already_at_target;
            result.migrated_payload = std::string(payload);
            result.migration_succeeded = true;
            result.payload_available = true;

            return result;
        }

        std::string current_payload(payload);
        std::uint32_t current_version = source_version;

        for (const auto& step : result.path.steps)
        {
            if (step.source_version != current_version)
            {
                result.status =
                    ExecutionStatus::version_contract_violation;

                return result;
            }

            if (!step.handler)
            {
                result.status =
                    ExecutionStatus::version_contract_violation;

                return result;
            }

            const auto step_result =
                step.handler(current_payload);

            if (!step_result.succeeded)
            {
                result.status = ExecutionStatus::handler_failed;
                result.diagnostic = step_result.diagnostic;

                return result;
            }

            if (step_result.payload.empty())
            {
                result.status =
                    ExecutionStatus::empty_handler_payload;

                result.diagnostic = step_result.diagnostic;

                return result;
            }

            current_payload = step_result.payload;
            current_version = step.target_version;
            ++result.completed_steps;
        }

        if (current_version != target_version)
        {
            result.status =
                ExecutionStatus::version_contract_violation;

            return result;
        }

        result.status = ExecutionStatus::migrated;
        result.migrated_payload = std::move(current_payload);
        result.migration_succeeded = true;
        result.payload_available = true;

        return result;
    }

    [[nodiscard]]
    bool Contains(
        const std::uint32_t source_version) const
    {
        return registrations_.find(source_version) !=
               registrations_.end();
    }

    [[nodiscard]]
    std::size_t Size() const noexcept
    {
        return registrations_.size();
    }

private:
    std::map<
        std::uint32_t,
        RuntimeServiceCapabilityTraceDistributionExplanationReportMigrationRegistration>
        registrations_{};
};

}
