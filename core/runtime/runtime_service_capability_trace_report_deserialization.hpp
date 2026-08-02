#pragma once

#include "runtime_service_capability_trace_query.hpp"

#include <cctype>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace sovereign
{

enum class RuntimeServiceCapabilityTraceDistributionExplanationReportValidationStatus
{
    valid,

    empty_payload,
    malformed_payload,
    unsupported_schema_version,
    invalid_shape,
    invalid_governing_rule,
    invalid_confidence,
    invalid_stability,
    invalid_boolean,
    invalid_unsigned_integer,
    numeric_value_out_of_range,
    inconsistent_report
};


struct RuntimeServiceCapabilityTraceDistributionExplanationReportDeserialization final
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportView report{};

    RuntimeServiceCapabilityTraceDistributionExplanationReportValidationStatus
        validation_status{
            RuntimeServiceCapabilityTraceDistributionExplanationReportValidationStatus::
                empty_payload};

    bool deserialization_available{false};
};


namespace detail
{

class RuntimeServiceCapabilityTraceCanonicalJsonReader final
{
public:
    explicit RuntimeServiceCapabilityTraceCanonicalJsonReader(
        const std::string_view payload) noexcept
        : payload_(payload)
    {
    }


    bool AtEnd() noexcept
    {
        SkipWhitespace();

        return position_ == payload_.size();
    }


    bool Consume(const char expected) noexcept
    {
        SkipWhitespace();

        if (
            position_ >= payload_.size() ||
            payload_[position_] != expected)
        {
            return false;
        }

        ++position_;

        return true;
    }


    bool ConsumeFieldName(
        const std::string_view name) noexcept
    {
        SkipWhitespace();

        if (!ConsumeRaw('"'))
        {
            return false;
        }

        if (
            payload_.substr(
                position_,
                name.size()) != name)
        {
            return false;
        }

        position_ += name.size();

        if (!ConsumeRaw('"'))
        {
            return false;
        }

        SkipWhitespace();

        return ConsumeRaw(':');
    }


    bool ReadString(std::string_view& value) noexcept
    {
        SkipWhitespace();

        if (!ConsumeRaw('"'))
        {
            return false;
        }

        const auto begin = position_;

        while (position_ < payload_.size())
        {
            const auto current = payload_[position_];

            if (current == '"')
            {
                value =
                    payload_.substr(
                        begin,
                        position_ - begin);

                ++position_;

                return true;
            }

            /*
             * RT-142 emits only canonical enum tokens.
             * Escaped or control characters are not valid in
             * this version of the wire contract.
             */
            if (
                current == '\\' ||
                static_cast<unsigned char>(current) < 0x20U)
            {
                return false;
            }

            ++position_;
        }

        return false;
    }


    bool ReadBoolean(bool& value) noexcept
    {
        SkipWhitespace();

        if (
            payload_.substr(
                position_,
                4U) == "true")
        {
            position_ += 4U;
            value = true;

            return true;
        }

        if (
            payload_.substr(
                position_,
                5U) == "false")
        {
            position_ += 5U;
            value = false;

            return true;
        }

        return false;
    }


    bool ReadUnsigned(
        std::uint64_t& value) noexcept
    {
        SkipWhitespace();

        if (
            position_ >= payload_.size() ||
            !std::isdigit(
                static_cast<unsigned char>(
                    payload_[position_])))
        {
            return false;
        }

        std::uint64_t parsed = 0U;

        while (
            position_ < payload_.size() &&
            std::isdigit(
                static_cast<unsigned char>(
                    payload_[position_])))
        {
            const auto digit =
                static_cast<std::uint64_t>(
                    payload_[position_] - '0');

            if (
                parsed >
                (
                    std::numeric_limits<std::uint64_t>::max() -
                    digit) /
                    10U)
            {
                return false;
            }

            parsed =
                (parsed * 10U) +
                digit;

            ++position_;
        }

        value = parsed;

        return true;
    }


private:
    void SkipWhitespace() noexcept
    {
        while (
            position_ < payload_.size() &&
            std::isspace(
                static_cast<unsigned char>(
                    payload_[position_])))
        {
            ++position_;
        }
    }


    bool ConsumeRaw(const char expected) noexcept
    {
        if (
            position_ >= payload_.size() ||
            payload_[position_] != expected)
        {
            return false;
        }

        ++position_;

        return true;
    }


    std::string_view payload_{};
    std::size_t position_{0U};
};


inline bool ParseRuntimeServiceCapabilityTraceDistributionShape(
    const std::string_view value,
    RuntimeServiceCapabilityTraceResultStatusDistributionShape&
        shape) noexcept
{
    using Shape =
        RuntimeServiceCapabilityTraceResultStatusDistributionShape;

    if (value == "empty")
    {
        shape = Shape::empty;
    }
    else if (value == "single_point")
    {
        shape = Shape::single_point;
    }
    else if (value == "collapsed_core_with_tails")
    {
        shape = Shape::collapsed_core_with_tails;
    }
    else if (value == "outlier_dominated")
    {
        shape = Shape::outlier_dominated;
    }
    else if (value == "left_skewed")
    {
        shape = Shape::left_skewed;
    }
    else if (value == "right_skewed")
    {
        shape = Shape::right_skewed;
    }
    else if (value == "compact_symmetric")
    {
        shape = Shape::compact_symmetric;
    }
    else if (value == "dispersed_symmetric")
    {
        shape = Shape::dispersed_symmetric;
    }
    else if (value == "ambiguous")
    {
        shape = Shape::ambiguous;
    }
    else
    {
        return false;
    }

    return true;
}


inline bool ParseRuntimeServiceCapabilityTraceDistributionShapeRule(
    const std::string_view value,
    RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule&
        rule) noexcept
{
    using Rule =
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule;

    if (value == "empty_distribution")
    {
        rule = Rule::empty_distribution;
    }
    else if (value == "zero_complete_spread")
    {
        rule = Rule::zero_complete_spread;
    }
    else if (value == "collapsed_interquartile_core")
    {
        rule = Rule::collapsed_interquartile_core;
    }
    else if (value == "dominant_outlier_coverage")
    {
        rule = Rule::dominant_outlier_coverage;
    }
    else if (value == "left_quartile_skew")
    {
        rule = Rule::left_quartile_skew;
    }
    else if (value == "right_quartile_skew")
    {
        rule = Rule::right_quartile_skew;
    }
    else if (value == "compact_quartile_symmetry")
    {
        rule = Rule::compact_quartile_symmetry;
    }
    else if (value == "dispersed_quartile_symmetry")
    {
        rule = Rule::dispersed_quartile_symmetry;
    }
    else if (value == "unresolved_distribution")
    {
        rule = Rule::unresolved_distribution;
    }
    else
    {
        return false;
    }

    return true;
}


inline bool ParseRuntimeServiceCapabilityTraceDistributionShapeConfidence(
    const std::string_view value,
    RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence&
        confidence) noexcept
{
    using Confidence =
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence;

    if (value == "undefined")
    {
        confidence = Confidence::undefined;
    }
    else if (value == "low")
    {
        confidence = Confidence::low;
    }
    else if (value == "moderate")
    {
        confidence = Confidence::moderate;
    }
    else if (value == "high")
    {
        confidence = Confidence::high;
    }
    else if (value == "definitive")
    {
        confidence = Confidence::definitive;
    }
    else
    {
        return false;
    }

    return true;
}


inline bool ParseRuntimeServiceCapabilityTraceDistributionExplanationStability(
    const std::string_view value,
    RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability&
        stability) noexcept
{
    using Stability =
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability;

    if (value == "undefined")
    {
        stability = Stability::undefined;
    }
    else if (value == "unstable")
    {
        stability = Stability::unstable;
    }
    else if (value == "conditionally_stable")
    {
        stability = Stability::conditionally_stable;
    }
    else if (value == "stable")
    {
        stability = Stability::stable;
    }
    else
    {
        return false;
    }

    return true;
}


template <typename Target>
inline bool AssignUnsigned(
    const std::uint64_t source,
    Target& target) noexcept
{
    if (
        source >
        static_cast<std::uint64_t>(
            std::numeric_limits<Target>::max()))
    {
        return false;
    }

    target = static_cast<Target>(source);

    return true;
}

} // namespace detail


inline RuntimeServiceCapabilityTraceDistributionExplanationReportDeserialization
DeserializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
    const std::string_view payload)
{
    using ValidationStatus =
        RuntimeServiceCapabilityTraceDistributionExplanationReportValidationStatus;

    RuntimeServiceCapabilityTraceDistributionExplanationReportDeserialization
        result;

    if (payload.empty())
    {
        result.validation_status =
            ValidationStatus::empty_payload;

        return result;
    }

    detail::RuntimeServiceCapabilityTraceCanonicalJsonReader
        reader(payload);

    RuntimeServiceCapabilityTraceDistributionExplanationReportView
        report;

    std::uint64_t unsigned_value = 0U;
    std::string_view string_value;
    std::uint64_t schema_version = 0U;

    const auto malformed =
        [&result]() noexcept
        {
            result.validation_status =
                ValidationStatus::malformed_payload;

            return result;
        };

    if (
        !reader.Consume('{') ||
        !reader.ConsumeFieldName("schema_version") ||
        !reader.ReadUnsigned(schema_version))
    {
        return malformed();
    }

    if (schema_version != 1U)
    {
        result.validation_status =
            ValidationStatus::unsupported_schema_version;

        return result;
    }

    if (
        !reader.Consume(',') ||
        !reader.ConsumeFieldName("total_traces") ||
        !reader.ReadUnsigned(unsigned_value))
    {
        return malformed();
    }

    if (
        !detail::AssignUnsigned(
            unsigned_value,
            report.total_traces))
    {
        result.validation_status =
            ValidationStatus::numeric_value_out_of_range;

        return result;
    }

    if (
        !reader.Consume(',') ||
        !reader.ConsumeFieldName("represented_statuses") ||
        !reader.ReadUnsigned(unsigned_value))
    {
        return malformed();
    }

    if (
        !detail::AssignUnsigned(
            unsigned_value,
            report.represented_statuses))
    {
        result.validation_status =
            ValidationStatus::numeric_value_out_of_range;

        return result;
    }

    if (
        !reader.Consume(',') ||
        !reader.ConsumeFieldName("shape") ||
        !reader.ReadString(string_value))
    {
        return malformed();
    }

    if (
        !detail::ParseRuntimeServiceCapabilityTraceDistributionShape(
            string_value,
            report.shape))
    {
        result.validation_status =
            ValidationStatus::invalid_shape;

        return result;
    }

    if (
        !reader.Consume(',') ||
        !reader.ConsumeFieldName("governing_rule") ||
        !reader.ReadString(string_value))
    {
        return malformed();
    }

    if (
        !detail::ParseRuntimeServiceCapabilityTraceDistributionShapeRule(
            string_value,
            report.governing_rule))
    {
        result.validation_status =
            ValidationStatus::invalid_governing_rule;

        return result;
    }

    if (
        !reader.Consume(',') ||
        !reader.ConsumeFieldName("confidence") ||
        !reader.ReadString(string_value))
    {
        return malformed();
    }

    if (
        !detail::ParseRuntimeServiceCapabilityTraceDistributionShapeConfidence(
            string_value,
            report.confidence))
    {
        result.validation_status =
            ValidationStatus::invalid_confidence;

        return result;
    }

    if (
        !reader.Consume(',') ||
        !reader.ConsumeFieldName("stability") ||
        !reader.ReadString(string_value))
    {
        return malformed();
    }

    if (
        !detail::ParseRuntimeServiceCapabilityTraceDistributionExplanationStability(
            string_value,
            report.stability))
    {
        result.validation_status =
            ValidationStatus::invalid_stability;

        return result;
    }

#define SOVEREIGN_RT143_READ_UNSIGNED_FIELD(field_name, destination)       \
    do                                                                     \
    {                                                                      \
        if (                                                               \
            !reader.Consume(',') ||                                        \
            !reader.ConsumeFieldName(field_name) ||                        \
            !reader.ReadUnsigned(unsigned_value))                          \
        {                                                                  \
            return malformed();                                            \
        }                                                                  \
                                                                           \
        if (                                                               \
            !detail::AssignUnsigned(                                       \
                unsigned_value,                                            \
                destination))                                              \
        {                                                                  \
            result.validation_status =                                     \
                ValidationStatus::numeric_value_out_of_range;              \
                                                                           \
            return result;                                                 \
        }                                                                  \
    } while (false)

#define SOVEREIGN_RT143_READ_BOOLEAN_FIELD(field_name, destination)        \
    do                                                                     \
    {                                                                      \
        if (                                                               \
            !reader.Consume(',') ||                                        \
            !reader.ConsumeFieldName(field_name))                          \
        {                                                                  \
            return malformed();                                            \
        }                                                                  \
                                                                           \
        if (!reader.ReadBoolean(destination))                              \
        {                                                                  \
            result.validation_status =                                     \
                ValidationStatus::invalid_boolean;                         \
                                                                           \
            return result;                                                 \
        }                                                                  \
    } while (false)

    SOVEREIGN_RT143_READ_UNSIGNED_FIELD(
        "confidence_basis_points",
        report.confidence_basis_points);

    SOVEREIGN_RT143_READ_UNSIGNED_FIELD(
        "evidence_margin_basis_points",
        report.evidence_margin_basis_points);

    SOVEREIGN_RT143_READ_UNSIGNED_FIELD(
        "stability_basis_points",
        report.stability_basis_points);

    SOVEREIGN_RT143_READ_UNSIGNED_FIELD(
        "conflicting_evidence_count",
        report.conflicting_evidence_count);

    SOVEREIGN_RT143_READ_BOOLEAN_FIELD(
        "stable",
        report.stable);

    SOVEREIGN_RT143_READ_BOOLEAN_FIELD(
        "conditionally_stable",
        report.conditionally_stable);

    SOVEREIGN_RT143_READ_BOOLEAN_FIELD(
        "unstable",
        report.unstable);

    SOVEREIGN_RT143_READ_BOOLEAN_FIELD(
        "structurally_anchored",
        report.structurally_anchored);

    SOVEREIGN_RT143_READ_BOOLEAN_FIELD(
        "threshold_sensitive",
        report.threshold_sensitive);

    SOVEREIGN_RT143_READ_BOOLEAN_FIELD(
        "evidence_sensitive",
        report.evidence_sensitive);

    SOVEREIGN_RT143_READ_BOOLEAN_FIELD(
        "sample_sensitive",
        report.sample_sensitive);

    SOVEREIGN_RT143_READ_BOOLEAN_FIELD(
        "fallback_sensitive",
        report.fallback_sensitive);

    SOVEREIGN_RT143_READ_BOOLEAN_FIELD(
        "matched_set_stable",
        report.matched_set_stable);

#undef SOVEREIGN_RT143_READ_UNSIGNED_FIELD
#undef SOVEREIGN_RT143_READ_BOOLEAN_FIELD

    if (
        !reader.Consume('}') ||
        !reader.AtEnd())
    {
        return malformed();
    }

    const auto stability_flag_count =
        static_cast<unsigned int>(report.stable) +
        static_cast<unsigned int>(report.conditionally_stable) +
        static_cast<unsigned int>(report.unstable);

    if (stability_flag_count != 1U)
    {
        result.validation_status =
            ValidationStatus::inconsistent_report;

        return result;
    }

    if (
        report.confidence_basis_points > 10000U ||
        report.evidence_margin_basis_points > 10000U ||
        report.stability_basis_points > 10000U)
    {
        result.validation_status =
            ValidationStatus::numeric_value_out_of_range;

        return result;
    }

    using Stability =
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability;

    const bool stability_consistent =
        (
            report.stability == Stability::stable &&
            report.stable &&
            !report.conditionally_stable &&
            !report.unstable
        ) ||
        (
            report.stability == Stability::conditionally_stable &&
            !report.stable &&
            report.conditionally_stable &&
            !report.unstable
        ) ||
        (
            report.stability == Stability::unstable &&
            !report.stable &&
            !report.conditionally_stable &&
            report.unstable
        );

    if (!stability_consistent)
    {
        result.validation_status =
            ValidationStatus::inconsistent_report;

        return result;
    }

    report.report_defined = true;

    result.report = report;
    result.validation_status =
        ValidationStatus::valid;
    result.deserialization_available = true;

    return result;
}

} // namespace sovereign
