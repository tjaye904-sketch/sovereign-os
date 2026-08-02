#pragma once

#include "runtime_service_capability_trace_report_deserialization.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace sovereign
{

enum class RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceStatus
{
    persisted,

    serialization_unavailable,
    invalid_path,
    directory_creation_failed,
    temporary_file_open_failed,
    temporary_file_write_failed,
    temporary_file_flush_failed,
    atomic_replace_failed
};


enum class RuntimeServiceCapabilityTraceDistributionExplanationReportLoadStatus
{
    loaded,

    invalid_path,
    file_not_found,
    file_open_failed,
    file_size_unavailable,
    payload_too_large,
    file_read_failed,
    validation_failed
};


struct RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceResult final
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceStatus
        status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceStatus::
                invalid_path};

    std::filesystem::path path{};

    std::size_t bytes_written{0U};

    bool persisted{false};
};


struct RuntimeServiceCapabilityTraceDistributionExplanationReportLoadResult final
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportLoadStatus
        status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportLoadStatus::
                invalid_path};

    RuntimeServiceCapabilityTraceDistributionExplanationReportDeserialization
        deserialization{};

    std::filesystem::path path{};

    std::size_t bytes_read{0U};

    bool loaded{false};
};


struct RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceOptions final
{
    bool create_parent_directories{true};

    std::size_t maximum_payload_bytes{
        1024U * 1024U};
};


namespace detail
{

inline std::filesystem::path
RuntimeServiceCapabilityTraceDistributionExplanationReportTemporaryPath(
    const std::filesystem::path& destination)
{
    auto temporary = destination;

    temporary += ".tmp";

    return temporary;
}


inline bool
RuntimeServiceCapabilityTraceDistributionExplanationReportPathIsUsable(
    const std::filesystem::path& path) noexcept
{
    return
        !path.empty() &&
        path.has_filename();
}

} // namespace detail


inline RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceResult
PersistRuntimeServiceCapabilityTraceDistributionExplanationReportSerialization(
    const RuntimeServiceCapabilityTraceDistributionExplanationReportSerialization&
        serialization,
    const std::filesystem::path& destination,
    const RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceOptions&
        options = {})
{
    using Status =
        RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceStatus;

    RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceResult
        result;

    result.path = destination;

    if (
        !detail::
            RuntimeServiceCapabilityTraceDistributionExplanationReportPathIsUsable(
                destination))
    {
        result.status = Status::invalid_path;

        return result;
    }

    if (!serialization.serialization_available)
    {
        result.status =
            Status::serialization_unavailable;

        return result;
    }

    const auto parent = destination.parent_path();

    if (
        options.create_parent_directories &&
        !parent.empty())
    {
        std::error_code directory_error;

        std::filesystem::create_directories(
            parent,
            directory_error);

        if (directory_error)
        {
            result.status =
                Status::directory_creation_failed;

            return result;
        }
    }

    const auto temporary =
        detail::
            RuntimeServiceCapabilityTraceDistributionExplanationReportTemporaryPath(
                destination);

    {
        std::ofstream stream(
            temporary,
            std::ios::binary |
                std::ios::trunc);

        if (!stream.is_open())
        {
            result.status =
                Status::temporary_file_open_failed;

            return result;
        }

        stream.write(
            serialization.payload.data(),
            static_cast<std::streamsize>(
                serialization.payload.size()));

        if (!stream.good())
        {
            stream.close();

            std::error_code cleanup_error;
            std::filesystem::remove(
                temporary,
                cleanup_error);

            result.status =
                Status::temporary_file_write_failed;

            return result;
        }

        stream.flush();

        if (!stream.good())
        {
            stream.close();

            std::error_code cleanup_error;
            std::filesystem::remove(
                temporary,
                cleanup_error);

            result.status =
                Status::temporary_file_flush_failed;

            return result;
        }
    }

    std::error_code replace_error;

    std::filesystem::rename(
        temporary,
        destination,
        replace_error);

    if (replace_error)
    {
        std::error_code cleanup_error;

        std::filesystem::remove(
            temporary,
            cleanup_error);

        result.status =
            Status::atomic_replace_failed;

        return result;
    }

    result.status = Status::persisted;
    result.bytes_written =
        serialization.payload.size();
    result.persisted = true;

    return result;
}


inline RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceResult
PersistRuntimeServiceCapabilityTraceDistributionExplanationReportView(
    const RuntimeServiceCapabilityTraceDistributionExplanationReportView&
        report,
    const std::filesystem::path& destination,
    const RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceOptions&
        options = {})
{
    return
        PersistRuntimeServiceCapabilityTraceDistributionExplanationReportSerialization(
            SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportView(
                report),
            destination,
            options);
}


inline RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceResult
PersistRuntimeServiceCapabilityTraceDistributionExplanationReport(
    const RuntimeServiceCapabilityTraceQuerySummary& summary,
    const std::filesystem::path& destination,
    const RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceOptions&
        options = {})
{
    return
        PersistRuntimeServiceCapabilityTraceDistributionExplanationReportSerialization(
            SerializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
                summary),
            destination,
            options);
}


inline RuntimeServiceCapabilityTraceDistributionExplanationReportLoadResult
LoadRuntimeServiceCapabilityTraceDistributionExplanationReport(
    const std::filesystem::path& source,
    const RuntimeServiceCapabilityTraceDistributionExplanationReportPersistenceOptions&
        options = {})
{
    using Status =
        RuntimeServiceCapabilityTraceDistributionExplanationReportLoadStatus;

    RuntimeServiceCapabilityTraceDistributionExplanationReportLoadResult
        result;

    result.path = source;

    if (
        !detail::
            RuntimeServiceCapabilityTraceDistributionExplanationReportPathIsUsable(
                source))
    {
        result.status = Status::invalid_path;

        return result;
    }

    std::error_code existence_error;

    const bool exists =
        std::filesystem::exists(
            source,
            existence_error);

    if (
        existence_error ||
        !exists)
    {
        result.status =
            Status::file_not_found;

        return result;
    }

    std::error_code size_error;

    const auto file_size =
        std::filesystem::file_size(
            source,
            size_error);

    if (size_error)
    {
        result.status =
            Status::file_size_unavailable;

        return result;
    }

    if (
        file_size >
        options.maximum_payload_bytes)
    {
        result.status =
            Status::payload_too_large;

        return result;
    }

    std::ifstream stream(
        source,
        std::ios::binary);

    if (!stream.is_open())
    {
        result.status =
            Status::file_open_failed;

        return result;
    }

    std::string payload(
        static_cast<std::size_t>(file_size),
        '\0');

    if (!payload.empty())
    {
        stream.read(
            payload.data(),
            static_cast<std::streamsize>(
                payload.size()));

        if (
            !stream ||
            static_cast<std::size_t>(
                stream.gcount()) !=
                payload.size())
        {
            result.status =
                Status::file_read_failed;

            return result;
        }
    }

    result.bytes_read = payload.size();

    result.deserialization =
        DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
            payload);

    if (
        !result.deserialization
             .deserialization_available)
    {
        result.status =
            Status::validation_failed;

        return result;
    }

    result.status = Status::loaded;
    result.loaded = true;

    return result;
}

} // namespace sovereign
