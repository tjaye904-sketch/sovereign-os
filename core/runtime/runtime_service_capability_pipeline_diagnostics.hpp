#pragma once

#include "uuid.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace sovereign
{

struct RuntimeServiceCapabilityPipelineTrace final
{
    Uuid trace_id{};

    std::uint64_t invocation_sequence{0U};

    std::string capability{};
    std::string preferred_service_name{};
    std::string operation{};

    bool request_policy_evaluated{false};

    std::uint8_t request_policy_status{0U};

    bool negotiation_attempted{false};

    std::uint8_t negotiation_status{0U};

    std::size_t candidate_count{0U};

    std::string selected_service_name{};

    bool provider_policy_evaluated{false};

    std::uint8_t provider_policy_status{0U};

    bool invocation_attempted{false};

    std::uint8_t invocation_status{0U};

    std::uint8_t result_status{0U};

    bool completed{false};

    std::uint64_t started_nanoseconds{0U};

    std::uint64_t request_policy_started_nanoseconds{0U};
    std::uint64_t request_policy_ended_nanoseconds{0U};

    std::uint64_t negotiation_started_nanoseconds{0U};
    std::uint64_t negotiation_ended_nanoseconds{0U};

    std::uint64_t provider_policy_started_nanoseconds{0U};
    std::uint64_t provider_policy_ended_nanoseconds{0U};

    std::uint64_t invocation_started_nanoseconds{0U};
    std::uint64_t invocation_ended_nanoseconds{0U};

    std::uint64_t ended_nanoseconds{0U};

    std::uint64_t request_policy_duration_nanoseconds{0U};
    std::uint64_t negotiation_duration_nanoseconds{0U};
    std::uint64_t provider_policy_duration_nanoseconds{0U};
    std::uint64_t invocation_duration_nanoseconds{0U};
    std::uint64_t total_duration_nanoseconds{0U};

    std::string diagnostic{};
};

} // namespace sovereign
