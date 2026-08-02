#pragma once

#include "runtime_service_capability_pipeline_diagnostics.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace sovereign
{

enum class RuntimeServiceCapabilityTraceSortOrder
{
    OldestFirst,
    NewestFirst
};

struct RuntimeServiceCapabilityTraceQuery
{
    std::optional<std::string> capability{};
    std::optional<std::string> operation{};
    std::optional<std::string> selected_service_name{};

    std::optional<bool> request_policy_evaluated{};
    std::optional<bool> negotiation_attempted{};
    std::optional<bool> provider_policy_evaluated{};
    std::optional<bool> invocation_attempted{};
    std::optional<bool> completed{};

    std::optional<std::uint8_t> result_status{};

    RuntimeServiceCapabilityTraceSortOrder sort_order{
        RuntimeServiceCapabilityTraceSortOrder::
            OldestFirst};

    /*
     * Number of ordered matching traces to skip before
     * collecting results.
     */
    std::size_t offset{0U};

    std::size_t maximum_results{
        std::numeric_limits<std::size_t>::max()};
};

struct RuntimeServiceCapabilityTraceResultStatusDistributionEntry
{
    std::uint8_t result_status{0U};
    std::size_t count{0U};
};


struct RuntimeServiceCapabilityTraceDistributionDiagnostics final
{
    std::size_t total_traces{0U};
    std::size_t represented_statuses{0U};

    std::size_t minimum_status_count{0U};
    std::size_t maximum_status_count{0U};

    bool empty{true};
    bool uniform{true};
};


struct RuntimeServiceCapabilityTraceDistributionStatistics final
{
    std::size_t total_traces{0U};
    std::size_t represented_statuses{0U};

    std::uint8_t minimum_result_status{0U};
    std::uint8_t maximum_result_status{0U};

    std::uint16_t result_status_span{0U};

    std::size_t dominant_trace_count{0U};
    std::size_t dominant_statuses{0U};

    bool empty{true};
    bool single_status{false};
};


struct RuntimeServiceCapabilityTraceDistributionStatisticalDiagnostics final
{
    std::size_t total_traces{0U};
    std::size_t represented_statuses{0U};

    std::uint16_t status_range_width{0U};
    std::size_t unrepresented_statuses_within_range{0U};

    std::uint16_t status_range_coverage_basis_points{0U};

    std::size_t dominant_trace_count{0U};
    std::size_t dominant_statuses{0U};

    bool empty{true};
    bool contiguous_status_range{false};
    bool tied_dominance{false};
    bool majority_dominance{false};
    bool sparse_status_range{false};
};


struct RuntimeServiceCapabilityTraceResultStatusPercentile final
{
    std::uint16_t requested_percentile_basis_points{0U};
    std::uint16_t effective_percentile_basis_points{0U};

    std::size_t total_traces{0U};
    std::size_t nearest_rank{0U};

    std::uint8_t result_status{0U};

    bool empty{true};
    bool percentile_clamped{false};
};



struct RuntimeServiceCapabilityTraceResultStatusOutlierDiagnostics final
{
    std::size_t total_traces{0U};
    std::size_t represented_statuses{0U};

    std::uint8_t first_quartile_result_status{0U};
    std::uint8_t third_quartile_result_status{0U};
    std::uint16_t interquartile_status_range{0U};

    /*
     * Exact Tukey fences represented at twice their status
     * value. This retains half-status boundaries without
     * floating-point arithmetic.
     */
    std::int16_t lower_fence_twice_status{0};
    std::int16_t upper_fence_twice_status{0};

    std::size_t lower_outlier_traces{0U};
    std::size_t upper_outlier_traces{0U};
    std::size_t total_outlier_traces{0U};

    std::size_t lower_outlier_statuses{0U};
    std::size_t upper_outlier_statuses{0U};
    std::size_t total_outlier_statuses{0U};

    std::uint16_t outlier_trace_coverage_basis_points{0U};

    bool empty{true};

    bool lower_outliers_present{false};
    bool upper_outliers_present{false};
    bool outliers_present{false};

    bool lower_fence_below_status_domain{false};
    bool upper_fence_above_status_domain{false};
};



enum class RuntimeServiceCapabilityTraceResultStatusDistributionShape
{
    empty,
    single_point,
    collapsed_core_with_tails,
    outlier_dominated,
    left_skewed,
    right_skewed,
    compact_symmetric,
    dispersed_symmetric,
    ambiguous
};


struct RuntimeServiceCapabilityTraceResultStatusDistributionShapeDiagnostics final
{
    std::size_t total_traces{0U};
    std::size_t represented_statuses{0U};

    std::uint8_t minimum_result_status{0U};
    std::uint8_t first_quartile_result_status{0U};
    std::uint8_t median_result_status{0U};
    std::uint8_t third_quartile_result_status{0U};
    std::uint8_t maximum_result_status{0U};

    std::uint16_t total_status_range{0U};
    std::uint16_t interquartile_status_range{0U};

    std::size_t total_outlier_traces{0U};

    std::uint16_t
        outlier_trace_coverage_basis_points{0U};

    std::int16_t skewness_basis_points{0};

    RuntimeServiceCapabilityTraceResultStatusDistributionShape
        shape{
            RuntimeServiceCapabilityTraceResultStatusDistributionShape::
                empty};

    bool empty{true};
    bool classified{false};

    bool single_point{false};
    bool collapsed_core_with_tails{false};
    bool outlier_dominated{false};

    bool left_skewed{false};
    bool right_skewed{false};

    bool compact_symmetric{false};
    bool dispersed_symmetric{false};

    bool ambiguous{false};
};

enum class RuntimeServiceCapabilityTraceResultStatusSkewDirection
{
    symmetric,
    left,
    right
};


enum class RuntimeServiceCapabilityTraceResultStatusSkewMagnitude
{
    none,
    slight,
    moderate,
    strong
};




enum class RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule
{
    empty_distribution,
    zero_complete_spread,
    collapsed_interquartile_core,
    dominant_outlier_coverage,
    left_quartile_skew,
    right_quartile_skew,
    compact_quartile_symmetry,
    dispersed_quartile_symmetry,
    unresolved_distribution
};


enum class RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence
{
    undefined,
    low,
    moderate,
    high,
    definitive
};


struct RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationDiagnostics final
{
    std::size_t total_traces{0U};
    std::size_t represented_statuses{0U};

    RuntimeServiceCapabilityTraceResultStatusDistributionShape
        shape{
            RuntimeServiceCapabilityTraceResultStatusDistributionShape::
                empty};

    RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence
        confidence{
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence::
                undefined};

    RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule
        governing_rule{
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
                empty_distribution};

    /*
     * RT-136 classification precedence.
     *
     * 1: empty distribution
     * 2: zero complete spread
     * 3: collapsed interquartile core
     * 4: dominant outlier coverage
     * 5: directional quartile skew
     * 6: quartile symmetry spread
     * 7: unresolved distribution
     */
    std::uint8_t precedence_rank{0U};

    std::uint16_t confidence_basis_points{0U};
    std::uint16_t evidence_margin_basis_points{0U};

    std::size_t conflicting_evidence_count{0U};

    bool explanation_available{false};

    bool structurally_conclusive{false};
    bool threshold_governed{false};
    bool directional_evidence{false};
    bool symmetry_evidence{false};
    bool fallback_classification{false};

    bool sample_limited{false};
    bool boundary_adjacent{false};
    bool conflicting_evidence{false};
    bool confidence_limited{false};

    bool matched_set_stable{false};
};


enum class RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability
{
    undefined,
    unstable,
    conditionally_stable,
    stable
};


struct RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStabilityDiagnostics final
{
    std::size_t total_traces{0U};
    std::size_t represented_statuses{0U};

    RuntimeServiceCapabilityTraceResultStatusDistributionShape
        shape{
            RuntimeServiceCapabilityTraceResultStatusDistributionShape::
                empty};

    RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule
        governing_rule{
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
                empty_distribution};

    RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence
        confidence{
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence::
                undefined};

    RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability
        stability{
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability::
                undefined};

    std::uint16_t stability_basis_points{0U};
    std::uint16_t confidence_basis_points{0U};
    std::uint16_t evidence_margin_basis_points{0U};

    std::size_t conflicting_evidence_count{0U};

    bool stability_defined{false};

    bool stable{false};
    bool conditionally_stable{false};
    bool unstable{false};

    bool structurally_anchored{false};
    bool threshold_sensitive{false};
    bool evidence_sensitive{false};
    bool sample_sensitive{false};
    bool fallback_sensitive{false};

    bool matched_set_stable{false};
};


struct RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidenceDiagnostics final
{
    std::size_t total_traces{0U};
    std::size_t represented_statuses{0U};

    RuntimeServiceCapabilityTraceResultStatusDistributionShape
        shape{
            RuntimeServiceCapabilityTraceResultStatusDistributionShape::
                empty};

    RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence
        confidence{
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence::
                undefined};

    /*
     * Overall confidence scaled to basis points.
     *
     *     0 represents undefined confidence.
     * 10000 represents definitive confidence.
     */
    std::uint16_t confidence_basis_points{0U};

    /*
     * Distance from the governing classification boundary,
     * scaled to basis points when a normalized boundary is
     * available.
     */
    std::uint16_t evidence_margin_basis_points{0U};

    std::uint16_t total_status_range{0U};
    std::uint16_t interquartile_status_range{0U};

    std::uint16_t
        outlier_trace_coverage_basis_points{0U};

    std::uint16_t absolute_skewness_basis_points{0U};

    std::size_t conflicting_evidence_count{0U};

    bool confidence_defined{false};

    bool sample_limited{false};
    bool boundary_adjacent{false};
    bool conflicting_evidence{false};

    bool low_confidence{false};
    bool moderate_confidence{false};
    bool high_confidence{false};
    bool definitive_confidence{false};
};


struct RuntimeServiceCapabilityTraceResultStatusSkewnessDiagnostics final
{
    std::size_t total_traces{0U};

    std::uint8_t first_quartile_result_status{0U};
    std::uint8_t median_result_status{0U};
    std::uint8_t third_quartile_result_status{0U};

    std::uint16_t interquartile_status_range{0U};

    /*
     * Bowley quartile-skewness numerator:
     *
     * Q3 + Q1 - (2 * median)
     */
    std::int16_t skewness_numerator{0};

    /*
     * Signed Bowley skewness scaled to basis points.
     *
     * -10000 represents maximal left skew.
     *      0 represents quartile symmetry.
     * +10000 represents maximal right skew.
     */
    std::int16_t skewness_basis_points{0};

    RuntimeServiceCapabilityTraceResultStatusSkewDirection
        direction{
            RuntimeServiceCapabilityTraceResultStatusSkewDirection::
                symmetric};

    RuntimeServiceCapabilityTraceResultStatusSkewMagnitude
        magnitude{
            RuntimeServiceCapabilityTraceResultStatusSkewMagnitude::
                none};

    bool empty{true};
    bool skewness_defined{false};

    bool symmetric{false};
    bool left_skewed{false};
    bool right_skewed{false};

    bool slight_skew{false};
    bool moderate_skew{false};
    bool strong_skew{false};
};

struct RuntimeServiceCapabilityTraceResultStatusQuartileDiagnostics final
{
    std::size_t total_traces{0U};

    std::uint8_t minimum_result_status{0U};
    std::uint8_t first_quartile_result_status{0U};
    std::uint8_t median_result_status{0U};
    std::uint8_t third_quartile_result_status{0U};
    std::uint8_t maximum_result_status{0U};

    std::uint16_t interquartile_status_range{0U};
    std::uint16_t lower_tail_status_range{0U};
    std::uint16_t upper_tail_status_range{0U};

    bool empty{true};
    bool zero_spread{false};

    bool lower_quartile_collapsed{false};
    bool interquartile_collapsed{false};
    bool upper_quartile_collapsed{false};
    bool all_quartiles_collapsed{false};
};

struct RuntimeServiceCapabilityTraceQuerySummary final
{
    /*
     * Completion state across every filtered match before
     * pagination is applied.
     */
    std::size_t completed_traces{0U};
    std::size_t incomplete_traces{0U};

    /*
     * Pipeline-stage participation across every filtered
     * match.
     */
    std::size_t request_policy_evaluated_traces{0U};
    std::size_t negotiation_attempted_traces{0U};
    std::size_t provider_policy_evaluated_traces{0U};
    std::size_t invocation_attempted_traces{0U};

    /*
     * Duration distribution across every filtered match.
     * Minimum and maximum remain zero when no traces match.
     */
    std::uint64_t total_duration_nanoseconds{0U};
    std::uint64_t minimum_duration_nanoseconds{0U};
    std::uint64_t maximum_duration_nanoseconds{0U};

    /*
     * Terminal result-status distribution across every
     * filtered match before pagination.
     *
     * The complete uint8_t domain is retained so future
     * status values remain observable without changing this
     * query result contract.
     */
    std::array<std::size_t, 256U>
        result_status_counts{};

    [[nodiscard]]
    std::size_t ResultStatusCount(
        const std::uint8_t status) const noexcept
    {
        return result_status_counts[
            static_cast<std::size_t>(status)];
    }

    [[nodiscard]]
    std::vector<
        RuntimeServiceCapabilityTraceResultStatusDistributionEntry>
    ResultStatusDistribution() const
    {
        std::vector<
            RuntimeServiceCapabilityTraceResultStatusDistributionEntry>
            distribution;

        for (
            std::size_t status = 0U;
            status < result_status_counts.size();
            ++status)
        {
            const auto count =
                result_status_counts[status];

            if (count == 0U)
            {
                continue;
            }

            distribution.push_back(
                RuntimeServiceCapabilityTraceResultStatusDistributionEntry{
                    static_cast<std::uint8_t>(status),
                    count});
        }

        return distribution;
    }
};


inline RuntimeServiceCapabilityTraceDistributionDiagnostics
DiagnoseRuntimeServiceCapabilityTraceResultStatusDistribution(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    RuntimeServiceCapabilityTraceDistributionDiagnostics
        diagnostics;

    const auto distribution =
        summary.ResultStatusDistribution();

    diagnostics.represented_statuses =
        distribution.size();

    diagnostics.empty =
        distribution.empty();

    if (distribution.empty())
    {
        return diagnostics;
    }

    bool first_entry = true;
    std::size_t reference_count = 0U;

    for (const auto& entry : distribution)
    {
        const auto& [result_status, trace_count] =
            entry;

        static_cast<void>(result_status);

        diagnostics.total_traces += trace_count;

        if (first_entry)
        {
            diagnostics.minimum_status_count =
                trace_count;

            diagnostics.maximum_status_count =
                trace_count;

            reference_count = trace_count;
            first_entry = false;
            continue;
        }

        if (
            trace_count <
            diagnostics.minimum_status_count)
        {
            diagnostics.minimum_status_count =
                trace_count;
        }

        if (
            trace_count >
            diagnostics.maximum_status_count)
        {
            diagnostics.maximum_status_count =
                trace_count;
        }

        if (trace_count != reference_count)
        {
            diagnostics.uniform = false;
        }
    }

    return diagnostics;
}


inline RuntimeServiceCapabilityTraceDistributionStatistics
AnalyzeRuntimeServiceCapabilityTraceResultStatusDistribution(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    RuntimeServiceCapabilityTraceDistributionStatistics
        statistics;

    const auto distribution =
        summary.ResultStatusDistribution();

    statistics.represented_statuses =
        distribution.size();

    statistics.empty =
        distribution.empty();

    statistics.single_status =
        distribution.size() == 1U;

    if (distribution.empty())
    {
        return statistics;
    }

    statistics.minimum_result_status =
        distribution.front().result_status;

    statistics.maximum_result_status =
        distribution.back().result_status;

    statistics.result_status_span =
        static_cast<std::uint16_t>(
            statistics.maximum_result_status) -
        static_cast<std::uint16_t>(
            statistics.minimum_result_status);

    for (const auto& entry : distribution)
    {
        statistics.total_traces +=
            entry.count;

        if (
            entry.count >
            statistics.dominant_trace_count)
        {
            statistics.dominant_trace_count =
                entry.count;

            statistics.dominant_statuses = 1U;
        }
        else if (
            entry.count ==
            statistics.dominant_trace_count)
        {
            ++statistics.dominant_statuses;
        }
    }

    return statistics;
}



inline RuntimeServiceCapabilityTraceResultStatusPercentile
AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
    const RuntimeServiceCapabilityTraceQuerySummary& summary,
    const std::uint16_t percentile_basis_points)
{
    RuntimeServiceCapabilityTraceResultStatusPercentile
        percentile;

    percentile.requested_percentile_basis_points =
        percentile_basis_points;

    percentile.effective_percentile_basis_points =
        percentile_basis_points > 10000U
            ? 10000U
            : percentile_basis_points;

    percentile.percentile_clamped =
        percentile.effective_percentile_basis_points !=
        percentile.requested_percentile_basis_points;

    const auto distribution =
        summary.ResultStatusDistribution();

    for (const auto& entry : distribution)
    {
        percentile.total_traces += entry.count;
    }

    percentile.empty =
        percentile.total_traces == 0U;

    if (percentile.empty)
    {
        return percentile;
    }

    /*
     * Nearest-rank percentile:
     *
     *     rank = ceil(percentile * count / 10000)
     *
     * The quotient-and-remainder form avoids multiplying
     * the complete trace count by the percentile value.
     * Percentile zero selects the first ordered trace.
     */
    if (
        percentile.effective_percentile_basis_points ==
        0U)
    {
        percentile.nearest_rank = 1U;
    }
    else
    {
        const std::size_t whole_groups =
            percentile.total_traces / 10000U;

        const std::size_t remainder =
            percentile.total_traces % 10000U;

        const std::size_t scaled_remainder =
            remainder *
            static_cast<std::size_t>(
                percentile
                    .effective_percentile_basis_points);

        percentile.nearest_rank =
            whole_groups *
                static_cast<std::size_t>(
                    percentile
                        .effective_percentile_basis_points) +
            scaled_remainder / 10000U +
            static_cast<std::size_t>(
                scaled_remainder % 10000U != 0U);
    }

    std::size_t cumulative_count = 0U;

    for (const auto& entry : distribution)
    {
        cumulative_count += entry.count;

        if (
            cumulative_count >=
            percentile.nearest_rank)
        {
            percentile.result_status =
                entry.result_status;

            return percentile;
        }
    }

    /*
     * The rank is bounded by total_traces, so this fallback
     * is defensive and preserves deterministic behavior if
     * the distribution contract changes.
     */
    percentile.result_status =
        distribution.back().result_status;

    return percentile;
}


inline RuntimeServiceCapabilityTraceResultStatusQuartileDiagnostics
DiagnoseRuntimeServiceCapabilityTraceResultStatusQuartiles(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    RuntimeServiceCapabilityTraceResultStatusQuartileDiagnostics
        diagnostics;

    const auto minimum =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            summary,
            0U);

    diagnostics.total_traces =
        minimum.total_traces;

    diagnostics.empty =
        minimum.empty;

    if (diagnostics.empty)
    {
        return diagnostics;
    }

    const auto first_quartile =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            summary,
            2500U);

    const auto median =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            summary,
            5000U);

    const auto third_quartile =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            summary,
            7500U);

    const auto maximum =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusPercentile(
            summary,
            10000U);

    diagnostics.minimum_result_status =
        minimum.result_status;

    diagnostics.first_quartile_result_status =
        first_quartile.result_status;

    diagnostics.median_result_status =
        median.result_status;

    diagnostics.third_quartile_result_status =
        third_quartile.result_status;

    diagnostics.maximum_result_status =
        maximum.result_status;

    diagnostics.interquartile_status_range =
        static_cast<std::uint16_t>(
            diagnostics.third_quartile_result_status) -
        static_cast<std::uint16_t>(
            diagnostics.first_quartile_result_status);

    diagnostics.lower_tail_status_range =
        static_cast<std::uint16_t>(
            diagnostics.first_quartile_result_status) -
        static_cast<std::uint16_t>(
            diagnostics.minimum_result_status);

    diagnostics.upper_tail_status_range =
        static_cast<std::uint16_t>(
            diagnostics.maximum_result_status) -
        static_cast<std::uint16_t>(
            diagnostics.third_quartile_result_status);

    diagnostics.zero_spread =
        diagnostics.minimum_result_status ==
        diagnostics.maximum_result_status;

    diagnostics.lower_quartile_collapsed =
        diagnostics.minimum_result_status ==
        diagnostics.first_quartile_result_status;

    diagnostics.interquartile_collapsed =
        diagnostics.first_quartile_result_status ==
        diagnostics.third_quartile_result_status;

    diagnostics.upper_quartile_collapsed =
        diagnostics.third_quartile_result_status ==
        diagnostics.maximum_result_status;

    diagnostics.all_quartiles_collapsed =
        diagnostics.first_quartile_result_status ==
            diagnostics.median_result_status &&
        diagnostics.median_result_status ==
            diagnostics.third_quartile_result_status;

    return diagnostics;
}



inline RuntimeServiceCapabilityTraceResultStatusSkewnessDiagnostics
DiagnoseRuntimeServiceCapabilityTraceResultStatusSkewness(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    RuntimeServiceCapabilityTraceResultStatusSkewnessDiagnostics
        diagnostics;

    const auto quartiles =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusQuartiles(
            summary);

    diagnostics.total_traces =
        quartiles.total_traces;

    diagnostics.empty =
        quartiles.empty;

    if (diagnostics.empty)
    {
        return diagnostics;
    }

    diagnostics.first_quartile_result_status =
        quartiles.first_quartile_result_status;

    diagnostics.median_result_status =
        quartiles.median_result_status;

    diagnostics.third_quartile_result_status =
        quartiles.third_quartile_result_status;

    diagnostics.interquartile_status_range =
        quartiles.interquartile_status_range;

    diagnostics.skewness_numerator =
        static_cast<std::int16_t>(
            static_cast<std::int16_t>(
                diagnostics
                    .third_quartile_result_status) +
            static_cast<std::int16_t>(
                diagnostics
                    .first_quartile_result_status) -
            static_cast<std::int16_t>(
                2 *
                static_cast<std::int16_t>(
                    diagnostics
                        .median_result_status)));

    /*
     * A collapsed interquartile range has no usable Bowley
     * denominator. Preserve the numerator for diagnostics,
     * but report skewness as undefined.
     */
    if (diagnostics.interquartile_status_range == 0U)
    {
        return diagnostics;
    }

    diagnostics.skewness_defined = true;

    const auto negative =
        diagnostics.skewness_numerator < 0;

    const auto absolute_numerator =
        static_cast<std::uint16_t>(
            negative
                ? -diagnostics.skewness_numerator
                : diagnostics.skewness_numerator);

    const auto denominator =
        diagnostics.interquartile_status_range;

    /*
     * The numerator cannot exceed the IQR for ordered
     * quartiles, so this multiplication remains bounded:
     *
     * 255 * 10000 = 2,550,000
     */
    const auto absolute_basis_points =
        static_cast<std::uint16_t>(
            (
                static_cast<std::uint32_t>(
                    absolute_numerator) *
                10000U) /
            static_cast<std::uint32_t>(
                denominator));

    diagnostics.skewness_basis_points =
        static_cast<std::int16_t>(
            negative
                ? -static_cast<std::int32_t>(
                      absolute_basis_points)
                : static_cast<std::int32_t>(
                      absolute_basis_points));

    if (absolute_basis_points < 1000U)
    {
        diagnostics.direction =
            RuntimeServiceCapabilityTraceResultStatusSkewDirection::
                symmetric;

        diagnostics.magnitude =
            RuntimeServiceCapabilityTraceResultStatusSkewMagnitude::
                none;

        diagnostics.symmetric = true;

        return diagnostics;
    }

    if (negative)
    {
        diagnostics.direction =
            RuntimeServiceCapabilityTraceResultStatusSkewDirection::
                left;

        diagnostics.left_skewed = true;
    }
    else
    {
        diagnostics.direction =
            RuntimeServiceCapabilityTraceResultStatusSkewDirection::
                right;

        diagnostics.right_skewed = true;
    }

    if (absolute_basis_points < 2500U)
    {
        diagnostics.magnitude =
            RuntimeServiceCapabilityTraceResultStatusSkewMagnitude::
                slight;

        diagnostics.slight_skew = true;
    }
    else if (absolute_basis_points < 5000U)
    {
        diagnostics.magnitude =
            RuntimeServiceCapabilityTraceResultStatusSkewMagnitude::
                moderate;

        diagnostics.moderate_skew = true;
    }
    else
    {
        diagnostics.magnitude =
            RuntimeServiceCapabilityTraceResultStatusSkewMagnitude::
                strong;

        diagnostics.strong_skew = true;
    }

    return diagnostics;
}

inline RuntimeServiceCapabilityTraceResultStatusOutlierDiagnostics
DiagnoseRuntimeServiceCapabilityTraceResultStatusOutliers(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    RuntimeServiceCapabilityTraceResultStatusOutlierDiagnostics
        diagnostics;

    const auto quartiles =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusQuartiles(
            summary);

    diagnostics.total_traces =
        quartiles.total_traces;

    diagnostics.empty =
        quartiles.empty;

    if (diagnostics.empty)
    {
        return diagnostics;
    }

    diagnostics.first_quartile_result_status =
        quartiles.first_quartile_result_status;

    diagnostics.third_quartile_result_status =
        quartiles.third_quartile_result_status;

    diagnostics.interquartile_status_range =
        quartiles.interquartile_status_range;

    diagnostics.lower_fence_twice_status =
        static_cast<std::int16_t>(
            2 *
            static_cast<std::int16_t>(
                diagnostics
                    .first_quartile_result_status)) -
        static_cast<std::int16_t>(
            3 *
            static_cast<std::int16_t>(
                diagnostics
                    .interquartile_status_range));

    diagnostics.upper_fence_twice_status =
        static_cast<std::int16_t>(
            2 *
            static_cast<std::int16_t>(
                diagnostics
                    .third_quartile_result_status)) +
        static_cast<std::int16_t>(
            3 *
            static_cast<std::int16_t>(
                diagnostics
                    .interquartile_status_range));

    diagnostics.lower_fence_below_status_domain =
        diagnostics.lower_fence_twice_status < 0;

    diagnostics.upper_fence_above_status_domain =
        diagnostics.upper_fence_twice_status > 510;

    const auto distribution =
        summary.ResultStatusDistribution();

    diagnostics.represented_statuses =
        distribution.size();

    for (const auto& entry : distribution)
    {
        const auto status_twice =
            static_cast<std::int16_t>(
                2 *
                static_cast<std::int16_t>(
                    entry.result_status));

        if (
            status_twice <
            diagnostics.lower_fence_twice_status)
        {
            diagnostics.lower_outlier_traces +=
                entry.count;

            ++diagnostics.lower_outlier_statuses;
        }
        else if (
            status_twice >
            diagnostics.upper_fence_twice_status)
        {
            diagnostics.upper_outlier_traces +=
                entry.count;

            ++diagnostics.upper_outlier_statuses;
        }
    }

    diagnostics.total_outlier_traces =
        diagnostics.lower_outlier_traces +
        diagnostics.upper_outlier_traces;

    diagnostics.total_outlier_statuses =
        diagnostics.lower_outlier_statuses +
        diagnostics.upper_outlier_statuses;

    diagnostics.lower_outliers_present =
        diagnostics.lower_outlier_traces != 0U;

    diagnostics.upper_outliers_present =
        diagnostics.upper_outlier_traces != 0U;

    diagnostics.outliers_present =
        diagnostics.total_outlier_traces != 0U;

    /*
     * Calculate floor(outliers * 10000 / total) without
     * multiplying size_t counters. The repeated modular
     * addition remains bounded below total_traces.
     */
    std::size_t remainder = 0U;
    std::uint16_t basis_points = 0U;

    const auto increment =
        diagnostics.total_outlier_traces;

    const auto modulus =
        diagnostics.total_traces;

    for (std::size_t index = 0U; index < 10000U; ++index)
    {
        if (remainder >= modulus - increment)
        {
            remainder -= modulus - increment;
            ++basis_points;
        }
        else
        {
            remainder += increment;
        }
    }

    diagnostics.outlier_trace_coverage_basis_points =
        basis_points;

    return diagnostics;
}

inline RuntimeServiceCapabilityTraceDistributionStatisticalDiagnostics
DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionStatistics(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    RuntimeServiceCapabilityTraceDistributionStatisticalDiagnostics
        diagnostics;

    const auto statistics =
        AnalyzeRuntimeServiceCapabilityTraceResultStatusDistribution(
            summary);

    diagnostics.total_traces =
        statistics.total_traces;

    diagnostics.represented_statuses =
        statistics.represented_statuses;

    diagnostics.dominant_trace_count =
        statistics.dominant_trace_count;

    diagnostics.dominant_statuses =
        statistics.dominant_statuses;

    diagnostics.empty =
        statistics.empty;

    if (statistics.empty)
    {
        return diagnostics;
    }

    diagnostics.status_range_width =
        static_cast<std::uint16_t>(
            statistics.result_status_span + 1U);

    diagnostics.unrepresented_statuses_within_range =
        static_cast<std::size_t>(
            diagnostics.status_range_width) -
        diagnostics.represented_statuses;

    diagnostics.status_range_coverage_basis_points =
        static_cast<std::uint16_t>(
            (
                diagnostics.represented_statuses *
                10000U) /
            static_cast<std::size_t>(
                diagnostics.status_range_width));

    diagnostics.contiguous_status_range =
        diagnostics.unrepresented_statuses_within_range ==
        0U;

    diagnostics.tied_dominance =
        diagnostics.dominant_statuses > 1U;

    diagnostics.majority_dominance =
        diagnostics.dominant_trace_count >
        diagnostics.total_traces / 2U;

    diagnostics.sparse_status_range =
        diagnostics.status_range_coverage_basis_points <
        5000U;

    return diagnostics;
}

struct RuntimeServiceCapabilityTraceQueryDiagnostics final
{
    /*
     * Detached copy of the effective typed query.
     */
    RuntimeServiceCapabilityTraceQuery
        effective_query{};

    /*
     * Number of populated filtering fields composed using
     * logical AND.
     */
    std::size_t active_filters{0U};
};

struct RuntimeServiceCapabilityTraceQueryExecutionDiagnostics final
{
    /*
     * Pagination coordinates supplied by the caller.
     */
    std::size_t requested_offset{0U};
    std::size_t requested_limit{0U};

    /*
     * Result-shaping counters before and after pagination.
     */
    std::size_t matched_traces{0U};
    std::size_t returned_traces{0U};
    std::size_t remaining_traces{0U};

    /*
     * True when the requested offset identifies an
     * available matching trace.
     */
    bool offset_in_range{false};

    /*
     * True when maximum_results is zero and the query
     * intentionally returns only metadata.
     */
    bool metadata_only{false};

    /*
     * True when no later matching trace page remains.
     */
    bool terminal_page{false};
};

struct RuntimeServiceCapabilityTraceQueryResult
{
    std::vector<
        RuntimeServiceCapabilityPipelineTrace>
        traces{};

    /*
     * Requested offset applied after filtering and
     * ordering.
     */
    std::size_t offset{};

    std::size_t matched_count{};
    std::size_t returned_count{};

    /*
     * True when additional matching traces remain after
     * the returned page.
     */
    bool has_more{};

    /*
     * True when the result does not contain every matching
     * trace because of either offset or maximum_results.
     */
    bool truncated{};

    RuntimeServiceCapabilityTraceQuerySummary
        summary{};

    RuntimeServiceCapabilityTraceQueryDiagnostics
        query_diagnostics{};

    RuntimeServiceCapabilityTraceQueryExecutionDiagnostics
        execution_diagnostics{};
};

[[nodiscard]]
inline bool
RuntimeServiceCapabilityTraceMatches(
    const RuntimeServiceCapabilityPipelineTrace&
        trace,
    const RuntimeServiceCapabilityTraceQuery&
        query)
{
    if (
        query.capability.has_value() &&
        trace.capability != *query.capability)
    {
        return false;
    }

    if (
        query.operation.has_value() &&
        trace.operation != *query.operation)
    {
        return false;
    }

    if (
        query.selected_service_name.has_value() &&
        trace.selected_service_name !=
            *query.selected_service_name)
    {
        return false;
    }

    if (
        query.request_policy_evaluated.has_value() &&
        trace.request_policy_evaluated !=
            *query.request_policy_evaluated)
    {
        return false;
    }

    if (
        query.negotiation_attempted.has_value() &&
        trace.negotiation_attempted !=
            *query.negotiation_attempted)
    {
        return false;
    }

    if (
        query.provider_policy_evaluated.has_value() &&
        trace.provider_policy_evaluated !=
            *query.provider_policy_evaluated)
    {
        return false;
    }

    if (
        query.invocation_attempted.has_value() &&
        trace.invocation_attempted !=
            *query.invocation_attempted)
    {
        return false;
    }

    if (
        query.completed.has_value() &&
        trace.completed != *query.completed)
    {
        return false;
    }

    if (
        query.result_status.has_value() &&
        trace.result_status != *query.result_status)
    {
        return false;
    }

    return true;
}

[[nodiscard]]
inline RuntimeServiceCapabilityTraceQueryResult
QueryRuntimeServiceCapabilityTraces(
    const std::deque<
        RuntimeServiceCapabilityPipelineTrace>&
            traces,
    const RuntimeServiceCapabilityTraceQuery&
        query)
{
    RuntimeServiceCapabilityTraceQueryResult result;

    result.offset = query.offset;

    result.query_diagnostics.effective_query =
        query;

    result.query_diagnostics.active_filters =
        static_cast<std::size_t>(
            query.capability.has_value()) +
        static_cast<std::size_t>(
            query.operation.has_value()) +
        static_cast<std::size_t>(
            query.selected_service_name.has_value()) +
        static_cast<std::size_t>(
            query.request_policy_evaluated.has_value()) +
        static_cast<std::size_t>(
            query.negotiation_attempted.has_value()) +
        static_cast<std::size_t>(
            query.provider_policy_evaluated.has_value()) +
        static_cast<std::size_t>(
            query.invocation_attempted.has_value()) +
        static_cast<std::size_t>(
            query.completed.has_value()) +
        static_cast<std::size_t>(
            query.result_status.has_value());

    const auto consider =
        [&result, &query](
            const RuntimeServiceCapabilityPipelineTrace&
                trace)
        {
            if (
                !RuntimeServiceCapabilityTraceMatches(
                    trace,
                    query))
            {
                return;
            }

            const std::size_t match_index =
                result.matched_count;

            ++result.matched_count;

            ++result.summary.result_status_counts[
                static_cast<std::size_t>(
                    trace.result_status)];

            if (trace.completed)
            {
                ++result.summary.completed_traces;
            }
            else
            {
                ++result.summary.incomplete_traces;
            }

            if (trace.request_policy_evaluated)
            {
                ++result.summary
                      .request_policy_evaluated_traces;
            }

            if (trace.negotiation_attempted)
            {
                ++result.summary
                      .negotiation_attempted_traces;
            }

            if (trace.provider_policy_evaluated)
            {
                ++result.summary
                      .provider_policy_evaluated_traces;
            }

            if (trace.invocation_attempted)
            {
                ++result.summary
                      .invocation_attempted_traces;
            }

            result.summary.total_duration_nanoseconds +=
                trace.total_duration_nanoseconds;

            if (
                result.matched_count == 1U ||
                trace.total_duration_nanoseconds <
                    result.summary
                        .minimum_duration_nanoseconds)
            {
                result.summary
                    .minimum_duration_nanoseconds =
                    trace.total_duration_nanoseconds;
            }

            if (
                result.matched_count == 1U ||
                trace.total_duration_nanoseconds >
                    result.summary
                        .maximum_duration_nanoseconds)
            {
                result.summary
                    .maximum_duration_nanoseconds =
                    trace.total_duration_nanoseconds;
            }

            if (match_index < query.offset)
            {
                return;
            }

            if (
                result.traces.size() <
                query.maximum_results)
            {
                result.traces.push_back(trace);
            }
        };

    if (
        query.sort_order ==
        RuntimeServiceCapabilityTraceSortOrder::
            OldestFirst)
    {
        for (const auto& trace : traces)
        {
            consider(trace);
        }
    }
    else
    {
        for (
            auto iterator = traces.rbegin();
            iterator != traces.rend();
            ++iterator)
        {
            consider(*iterator);
        }
    }

    result.returned_count =
        result.traces.size();

    const std::size_t matches_after_offset =
        query.offset < result.matched_count
            ? result.matched_count - query.offset
            : 0U;

    result.has_more =
        result.returned_count <
        matches_after_offset;

    result.truncated =
        query.offset > 0U ||
        result.has_more;

    result.execution_diagnostics =
        RuntimeServiceCapabilityTraceQueryExecutionDiagnostics{
            .requested_offset = query.offset,
            .requested_limit =
                query.maximum_results,
            .matched_traces =
                result.matched_count,
            .returned_traces =
                result.returned_count,
            .remaining_traces =
                matches_after_offset -
                result.returned_count,
            .offset_in_range =
                query.offset <
                result.matched_count,
            .metadata_only =
                query.maximum_results == 0U,
            .terminal_page =
                !result.has_more};

    return result;
}

[[nodiscard]]
inline std::optional<
    RuntimeServiceCapabilityPipelineTrace>
FindRuntimeServiceCapabilityTrace(
    const std::deque<
        RuntimeServiceCapabilityPipelineTrace>&
            traces,
    const Uuid& trace_id)
{
    if (trace_id.IsNil())
    {
        return std::nullopt;
    }

    for (const auto& trace : traces)
    {
        if (trace.trace_id == trace_id)
        {
            return trace;
        }
    }

    return std::nullopt;
}


inline RuntimeServiceCapabilityTraceResultStatusDistributionShapeDiagnostics
DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    RuntimeServiceCapabilityTraceResultStatusDistributionShapeDiagnostics
        diagnostics;

    const auto quartiles =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusQuartiles(
            summary);

    const auto outliers =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusOutliers(
            summary);

    const auto skewness =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusSkewness(
            summary);

    diagnostics.total_traces =
        quartiles.total_traces;

    diagnostics.represented_statuses =
        outliers.represented_statuses;

    diagnostics.minimum_result_status =
        quartiles.minimum_result_status;

    diagnostics.first_quartile_result_status =
        quartiles.first_quartile_result_status;

    diagnostics.median_result_status =
        quartiles.median_result_status;

    diagnostics.third_quartile_result_status =
        quartiles.third_quartile_result_status;

    diagnostics.maximum_result_status =
        quartiles.maximum_result_status;

    diagnostics.total_status_range =
        static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(
                diagnostics.maximum_result_status) -
            static_cast<std::uint16_t>(
                diagnostics.minimum_result_status));

    diagnostics.interquartile_status_range =
        quartiles.interquartile_status_range;

    diagnostics.total_outlier_traces =
        outliers.total_outlier_traces;

    diagnostics.outlier_trace_coverage_basis_points =
        outliers.outlier_trace_coverage_basis_points;

    diagnostics.skewness_basis_points =
        skewness.skewness_basis_points;

    diagnostics.empty =
        quartiles.empty;

    if (diagnostics.empty)
    {
        diagnostics.shape =
            RuntimeServiceCapabilityTraceResultStatusDistributionShape::
                empty;

        diagnostics.classified = true;

        return diagnostics;
    }

    if (quartiles.zero_spread)
    {
        diagnostics.shape =
            RuntimeServiceCapabilityTraceResultStatusDistributionShape::
                single_point;

        diagnostics.classified = true;
        diagnostics.single_point = true;

        return diagnostics;
    }

    /*
     * A collapsed IQR with nonzero complete spread represents
     * a concentrated central mass with observations in one
     * or both tails.
     */
    if (quartiles.interquartile_status_range == 0U)
    {
        diagnostics.shape =
            RuntimeServiceCapabilityTraceResultStatusDistributionShape::
                collapsed_core_with_tails;

        diagnostics.classified = true;
        diagnostics.collapsed_core_with_tails = true;

        return diagnostics;
    }

    /*
     * Outliers take precedence once at least one quarter of
     * matched traces are outside the Tukey fences.
     */
    if (
        outliers.outlier_trace_coverage_basis_points >=
        2500U)
    {
        diagnostics.shape =
            RuntimeServiceCapabilityTraceResultStatusDistributionShape::
                outlier_dominated;

        diagnostics.classified = true;
        diagnostics.outlier_dominated = true;

        return diagnostics;
    }

    if (skewness.left_skewed)
    {
        diagnostics.shape =
            RuntimeServiceCapabilityTraceResultStatusDistributionShape::
                left_skewed;

        diagnostics.classified = true;
        diagnostics.left_skewed = true;

        return diagnostics;
    }

    if (skewness.right_skewed)
    {
        diagnostics.shape =
            RuntimeServiceCapabilityTraceResultStatusDistributionShape::
                right_skewed;

        diagnostics.classified = true;
        diagnostics.right_skewed = true;

        return diagnostics;
    }

    if (skewness.symmetric)
    {
        const auto twice_interquartile_range =
            static_cast<std::uint32_t>(
                diagnostics.interquartile_status_range) *
            2U;

        if (
            twice_interquartile_range <=
            static_cast<std::uint32_t>(
                diagnostics.total_status_range))
        {
            diagnostics.shape =
                RuntimeServiceCapabilityTraceResultStatusDistributionShape::
                    compact_symmetric;

            diagnostics.compact_symmetric = true;
        }
        else
        {
            diagnostics.shape =
                RuntimeServiceCapabilityTraceResultStatusDistributionShape::
                    dispersed_symmetric;

            diagnostics.dispersed_symmetric = true;
        }

        diagnostics.classified = true;

        return diagnostics;
    }

    diagnostics.shape =
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            ambiguous;

    diagnostics.classified = true;
    diagnostics.ambiguous = true;

    return diagnostics;
}


inline RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidenceDiagnostics
DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidenceDiagnostics
        diagnostics;

    const auto shape =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            summary);

    diagnostics.total_traces =
        shape.total_traces;

    diagnostics.represented_statuses =
        shape.represented_statuses;

    diagnostics.shape =
        shape.shape;

    diagnostics.total_status_range =
        shape.total_status_range;

    diagnostics.interquartile_status_range =
        shape.interquartile_status_range;

    diagnostics.outlier_trace_coverage_basis_points =
        shape.outlier_trace_coverage_basis_points;

    const auto signed_skewness =
        static_cast<std::int32_t>(
            shape.skewness_basis_points);

    diagnostics.absolute_skewness_basis_points =
        static_cast<std::uint16_t>(
            signed_skewness < 0
                ? -signed_skewness
                : signed_skewness);

    if (shape.empty)
    {
        return diagnostics;
    }

    std::uint16_t confidence_basis_points = 0U;

    if (shape.single_point)
    {
        /*
         * Zero complete spread is structurally conclusive,
         * independent of general sample-size confidence.
         */
        confidence_basis_points = 10000U;

        diagnostics.evidence_margin_basis_points =
            10000U;
    }
    else if (shape.collapsed_core_with_tails)
    {
        confidence_basis_points = 7500U;

        diagnostics.evidence_margin_basis_points =
            10000U;
    }
    else if (shape.outlier_dominated)
    {
        const auto threshold_margin =
            static_cast<std::uint16_t>(
                diagnostics
                    .outlier_trace_coverage_basis_points -
                2500U);

        diagnostics.evidence_margin_basis_points =
            threshold_margin;

        const auto candidate =
            static_cast<std::uint32_t>(7500U) +
            static_cast<std::uint32_t>(
                threshold_margin);

        confidence_basis_points =
            static_cast<std::uint16_t>(
                candidate > 10000U
                    ? 10000U
                    : candidate);

        diagnostics.boundary_adjacent =
            threshold_margin == 0U;
    }
    else if (
        shape.left_skewed ||
        shape.right_skewed)
    {
        const auto skewness_margin =
            diagnostics.absolute_skewness_basis_points >
                    1000U
                ? static_cast<std::uint16_t>(
                      diagnostics
                          .absolute_skewness_basis_points -
                      1000U)
                : 0U;

        diagnostics.evidence_margin_basis_points =
            static_cast<std::uint16_t>(
                skewness_margin);

        const auto candidate =
            static_cast<std::uint32_t>(5000U) +
            (
                static_cast<std::uint32_t>(
                    diagnostics
                        .absolute_skewness_basis_points) /
                2U);

        confidence_basis_points =
            static_cast<std::uint16_t>(
                candidate > 10000U
                    ? 10000U
                    : candidate);

        diagnostics.boundary_adjacent =
            skewness_margin == 0U;
    }
    else if (
        shape.compact_symmetric ||
        shape.dispersed_symmetric)
    {
        const auto twice_interquartile_range =
            static_cast<std::uint32_t>(
                diagnostics.interquartile_status_range) *
            2U;

        const auto complete_range =
            static_cast<std::uint32_t>(
                diagnostics.total_status_range);

        const auto absolute_boundary_distance =
            twice_interquartile_range > complete_range
                ? twice_interquartile_range -
                      complete_range
                : complete_range -
                      twice_interquartile_range;

        std::uint16_t normalized_margin = 0U;

        if (complete_range != 0U)
        {
            const auto candidate =
                (
                    absolute_boundary_distance *
                    10000U) /
                complete_range;

            normalized_margin =
                static_cast<std::uint16_t>(
                    candidate > 10000U
                        ? 10000U
                        : candidate);
        }

        diagnostics.evidence_margin_basis_points =
            normalized_margin;

        confidence_basis_points =
            static_cast<std::uint16_t>(
                5000U +
                (
                    static_cast<std::uint32_t>(
                        normalized_margin) /
                    2U));

        diagnostics.boundary_adjacent =
            normalized_margin == 0U;
    }
    else
    {
        confidence_basis_points = 2500U;

        diagnostics.evidence_margin_basis_points =
            0U;

        diagnostics.boundary_adjacent = true;

        diagnostics.conflicting_evidence_count =
            1U;
    }

    /*
     * Nondominant outliers are conflicting evidence for
     * classifications governed by skewness or symmetry.
     */
    if (
        !shape.outlier_dominated &&
        !shape.collapsed_core_with_tails &&
        shape.total_outlier_traces != 0U)
    {
        ++diagnostics.conflicting_evidence_count;
    }

    /*
     * A nonzero subthreshold skew is conflicting evidence
     * for an otherwise symmetric classification.
     */
    if (
        (
            shape.compact_symmetric ||
            shape.dispersed_symmetric) &&
        diagnostics.absolute_skewness_basis_points != 0U)
    {
        ++diagnostics.conflicting_evidence_count;
    }

    diagnostics.conflicting_evidence =
        diagnostics.conflicting_evidence_count != 0U;

    /*
     * General distribution confidence requires sufficient
     * matched-set support. Structural single-point evidence
     * is exempt because zero complete spread is conclusive.
     */
    if (!shape.single_point)
    {
        std::uint16_t sample_ceiling = 10000U;

        if (diagnostics.total_traces < 4U)
        {
            sample_ceiling = 2500U;
        }
        else if (diagnostics.total_traces < 8U)
        {
            sample_ceiling = 5000U;
        }

        if (confidence_basis_points > sample_ceiling)
        {
            confidence_basis_points =
                sample_ceiling;

            diagnostics.sample_limited = true;
        }
    }

    diagnostics.confidence_basis_points =
        confidence_basis_points;

    diagnostics.confidence_defined = true;

    if (confidence_basis_points == 10000U)
    {
        diagnostics.confidence =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence::
                definitive;

        diagnostics.definitive_confidence = true;
    }
    else if (confidence_basis_points > 5000U)
    {
        diagnostics.confidence =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence::
                high;

        diagnostics.high_confidence = true;
    }
    else if (confidence_basis_points > 2500U)
    {
        diagnostics.confidence =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence::
                moderate;

        diagnostics.moderate_confidence = true;
    }
    else
    {
        diagnostics.confidence =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence::
                low;

        diagnostics.low_confidence = true;
    }

    return diagnostics;
}


inline RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationDiagnostics
DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationDiagnostics
        diagnostics;

    const auto shape =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShape(
            summary);

    const auto confidence =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence(
            summary);

    diagnostics.total_traces =
        shape.total_traces;

    diagnostics.represented_statuses =
        shape.represented_statuses;

    diagnostics.shape =
        shape.shape;

    diagnostics.confidence =
        confidence.confidence;

    diagnostics.confidence_basis_points =
        confidence.confidence_basis_points;

    diagnostics.evidence_margin_basis_points =
        confidence.evidence_margin_basis_points;

    diagnostics.conflicting_evidence_count =
        confidence.conflicting_evidence_count;

    diagnostics.sample_limited =
        confidence.sample_limited;

    diagnostics.boundary_adjacent =
        confidence.boundary_adjacent;

    diagnostics.conflicting_evidence =
        confidence.conflicting_evidence;

    diagnostics.confidence_limited =
        diagnostics.sample_limited ||
        diagnostics.boundary_adjacent ||
        diagnostics.conflicting_evidence;

    /*
     * These diagnostics are computed exclusively from the
     * matched-set summary and are therefore unaffected by
     * returned-record pagination or metadata-only shaping.
     */
    diagnostics.matched_set_stable = true;

    if (shape.empty)
    {
        diagnostics.governing_rule =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
                empty_distribution;

        diagnostics.precedence_rank = 1U;
        diagnostics.structurally_conclusive = true;
        diagnostics.explanation_available = true;

        return diagnostics;
    }

    if (shape.single_point)
    {
        diagnostics.governing_rule =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
                zero_complete_spread;

        diagnostics.precedence_rank = 2U;
        diagnostics.structurally_conclusive = true;
        diagnostics.explanation_available = true;

        return diagnostics;
    }

    if (shape.collapsed_core_with_tails)
    {
        diagnostics.governing_rule =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
                collapsed_interquartile_core;

        diagnostics.precedence_rank = 3U;
        diagnostics.structurally_conclusive = true;
        diagnostics.explanation_available = true;

        return diagnostics;
    }

    if (shape.outlier_dominated)
    {
        diagnostics.governing_rule =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
                dominant_outlier_coverage;

        diagnostics.precedence_rank = 4U;
        diagnostics.threshold_governed = true;
        diagnostics.explanation_available = true;

        return diagnostics;
    }

    if (shape.left_skewed)
    {
        diagnostics.governing_rule =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
                left_quartile_skew;

        diagnostics.precedence_rank = 5U;
        diagnostics.directional_evidence = true;
        diagnostics.explanation_available = true;

        return diagnostics;
    }

    if (shape.right_skewed)
    {
        diagnostics.governing_rule =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
                right_quartile_skew;

        diagnostics.precedence_rank = 5U;
        diagnostics.directional_evidence = true;
        diagnostics.explanation_available = true;

        return diagnostics;
    }

    if (shape.compact_symmetric)
    {
        diagnostics.governing_rule =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
                compact_quartile_symmetry;

        diagnostics.precedence_rank = 6U;
        diagnostics.symmetry_evidence = true;
        diagnostics.explanation_available = true;

        return diagnostics;
    }

    if (shape.dispersed_symmetric)
    {
        diagnostics.governing_rule =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
                dispersed_quartile_symmetry;

        diagnostics.precedence_rank = 6U;
        diagnostics.symmetry_evidence = true;
        diagnostics.explanation_available = true;

        return diagnostics;
    }

    diagnostics.governing_rule =
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
            unresolved_distribution;

    diagnostics.precedence_rank = 7U;
    diagnostics.fallback_classification = true;
    diagnostics.explanation_available = true;

    return diagnostics;
}


inline RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStabilityDiagnostics
DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStabilityDiagnostics
        diagnostics;

    const auto explanation =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
            summary);

    diagnostics.total_traces =
        explanation.total_traces;

    diagnostics.represented_statuses =
        explanation.represented_statuses;

    diagnostics.shape =
        explanation.shape;

    diagnostics.governing_rule =
        explanation.governing_rule;

    diagnostics.confidence =
        explanation.confidence;

    diagnostics.confidence_basis_points =
        explanation.confidence_basis_points;

    diagnostics.evidence_margin_basis_points =
        explanation.evidence_margin_basis_points;

    diagnostics.conflicting_evidence_count =
        explanation.conflicting_evidence_count;

    diagnostics.matched_set_stable =
        explanation.matched_set_stable;

    if (!explanation.explanation_available)
    {
        return diagnostics;
    }

    diagnostics.stability_defined = true;

    /*
     * Structural rules are invariant with respect to the
     * confidence limitations applied to general statistical
     * classifications.
     */
    if (explanation.structurally_conclusive)
    {
        diagnostics.stability =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability::
                stable;

        diagnostics.stability_basis_points = 10000U;
        diagnostics.structurally_anchored = true;
        diagnostics.stable = true;

        return diagnostics;
    }

    /*
     * Fallback classifications and exact decision-boundary
     * classifications are directly sensitive to small changes
     * in the matched distribution.
     */
    if (
        explanation.fallback_classification ||
        explanation.boundary_adjacent)
    {
        diagnostics.stability =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability::
                unstable;

        diagnostics.stability_basis_points = 0U;
        diagnostics.threshold_sensitive =
            explanation.boundary_adjacent;

        diagnostics.fallback_sensitive =
            explanation.fallback_classification;

        diagnostics.unstable = true;

        return diagnostics;
    }

    /*
     * Conflicting evidence means that more than one shape
     * tendency is materially represented in the matched set.
     */
    if (
        explanation.conflicting_evidence &&
        explanation.confidence_basis_points < 7500U)
    {
        diagnostics.stability =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability::
                unstable;

        diagnostics.stability_basis_points =
            explanation.evidence_margin_basis_points > 2500U
                ? 2500U
                : explanation.evidence_margin_basis_points;

        diagnostics.evidence_sensitive = true;
        diagnostics.unstable = true;

        return diagnostics;
    }

    /*
     * Sample-limited classifications retain their current
     * explanation but lack enough matched-set support to be
     * treated as fully stable.
     */
    if (explanation.sample_limited)
    {
        diagnostics.stability =
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability::
                conditionally_stable;

        diagnostics.stability_basis_points =
            explanation.confidence_basis_points;

        diagnostics.sample_sensitive = true;
        diagnostics.conditionally_stable = true;

        return diagnostics;
    }

    diagnostics.stability =
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability::
            stable;

    diagnostics.stability_basis_points =
        explanation.confidence_basis_points;

    diagnostics.stable = true;

    return diagnostics;
}



struct RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStabilityReport final
{
    RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationDiagnostics
        explanation{};

    RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStabilityDiagnostics
        stability{};

    std::uint16_t overall_basis_points{0U};

    bool report_available{false};
    bool explanation_reliable{false};
    bool stability_reliable{false};
};


inline RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStabilityReport
BuildRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStabilityReport(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStabilityReport
        report;

    report.explanation =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanation(
            summary);

    report.stability =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability(
            summary);

    report.report_available =
        report.explanation.explanation_available &&
        report.stability.stability_defined;

    report.explanation_reliable =
        report.explanation.explanation_available;

    report.stability_reliable =
        report.stability.stability_defined;

    if (!report.report_available)
    {
        return report;
    }

    report.overall_basis_points =
        report.stability.stability_basis_points;

    return report;
}


struct RuntimeServiceCapabilityTraceDistributionExplanationReportView final
{
    std::size_t total_traces{0U};
    std::size_t represented_statuses{0U};

    RuntimeServiceCapabilityTraceResultStatusDistributionShape shape{
        RuntimeServiceCapabilityTraceResultStatusDistributionShape::
            empty};

    RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule
        governing_rule{
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule::
                empty_distribution};

    RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence
        confidence{
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence::
                undefined};

    RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability
        stability{
            RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability::
                undefined};

    std::uint16_t confidence_basis_points{0U};
    std::uint16_t evidence_margin_basis_points{0U};
    std::uint16_t stability_basis_points{0U};

    std::size_t conflicting_evidence_count{0U};

    bool report_defined{false};

    bool stable{false};
    bool conditionally_stable{false};
    bool unstable{false};

    bool structurally_anchored{false};
    bool threshold_sensitive{false};
    bool evidence_sensitive{false};
    bool sample_sensitive{false};
    bool fallback_sensitive{false};

    bool matched_set_stable{false};
};


inline RuntimeServiceCapabilityTraceDistributionExplanationReportView
BuildRuntimeServiceCapabilityTraceDistributionExplanationReportView(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportView
        report;

    const auto stability =
        DiagnoseRuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability(
            summary);

    report.total_traces =
        stability.total_traces;

    report.represented_statuses =
        stability.represented_statuses;

    report.shape =
        stability.shape;

    report.governing_rule =
        stability.governing_rule;

    report.confidence =
        stability.confidence;

    report.stability =
        stability.stability;

    report.confidence_basis_points =
        stability.confidence_basis_points;

    report.evidence_margin_basis_points =
        stability.evidence_margin_basis_points;

    report.stability_basis_points =
        stability.stability_basis_points;

    report.conflicting_evidence_count =
        stability.conflicting_evidence_count;

    report.report_defined =
        stability.stability_defined;

    report.stable =
        stability.stable;

    report.conditionally_stable =
        stability.conditionally_stable;

    report.unstable =
        stability.unstable;

    report.structurally_anchored =
        stability.structurally_anchored;

    report.threshold_sensitive =
        stability.threshold_sensitive;

    report.evidence_sensitive =
        stability.evidence_sensitive;

    report.sample_sensitive =
        stability.sample_sensitive;

    report.fallback_sensitive =
        stability.fallback_sensitive;

    report.matched_set_stable =
        stability.matched_set_stable;

    return report;
}


struct RuntimeServiceCapabilityTraceDistributionExplanationReportSerialization final
{
    std::uint16_t schema_version{1U};

    std::string payload{};

    bool serialization_available{false};
    bool matched_set_stable{false};
};


inline const char*
RuntimeServiceCapabilityTraceDistributionShapeName(
    const RuntimeServiceCapabilityTraceResultStatusDistributionShape
        shape) noexcept
{
    using Shape =
        RuntimeServiceCapabilityTraceResultStatusDistributionShape;

    switch (shape)
    {
        case Shape::empty:
            return "empty";

        case Shape::single_point:
            return "single_point";

        case Shape::collapsed_core_with_tails:
            return "collapsed_core_with_tails";

        case Shape::outlier_dominated:
            return "outlier_dominated";

        case Shape::left_skewed:
            return "left_skewed";

        case Shape::right_skewed:
            return "right_skewed";

        case Shape::compact_symmetric:
            return "compact_symmetric";

        case Shape::dispersed_symmetric:
            return "dispersed_symmetric";

        case Shape::ambiguous:
            return "ambiguous";
    }

    return "unknown";
}


inline const char*
RuntimeServiceCapabilityTraceDistributionShapeRuleName(
    const RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule
        rule) noexcept
{
    using Rule =
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeRule;

    switch (rule)
    {
        case Rule::empty_distribution:
            return "empty_distribution";

        case Rule::zero_complete_spread:
            return "zero_complete_spread";

        case Rule::collapsed_interquartile_core:
            return "collapsed_interquartile_core";

        case Rule::dominant_outlier_coverage:
            return "dominant_outlier_coverage";

        case Rule::left_quartile_skew:
            return "left_quartile_skew";

        case Rule::right_quartile_skew:
            return "right_quartile_skew";

        case Rule::compact_quartile_symmetry:
            return "compact_quartile_symmetry";

        case Rule::dispersed_quartile_symmetry:
            return "dispersed_quartile_symmetry";

        case Rule::unresolved_distribution:
            return "unresolved_distribution";
    }

    return "unknown";
}


inline const char*
RuntimeServiceCapabilityTraceDistributionShapeConfidenceName(
    const RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence
        confidence) noexcept
{
    using Confidence =
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeConfidence;

    switch (confidence)
    {
        case Confidence::undefined:
            return "undefined";

        case Confidence::low:
            return "low";

        case Confidence::moderate:
            return "moderate";

        case Confidence::high:
            return "high";

        case Confidence::definitive:
            return "definitive";
    }

    return "unknown";
}


inline const char*
RuntimeServiceCapabilityTraceDistributionExplanationStabilityName(
    const RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability
        stability) noexcept
{
    using Stability =
        RuntimeServiceCapabilityTraceResultStatusDistributionShapeExplanationStability;

    switch (stability)
    {
        case Stability::undefined:
            return "undefined";

        case Stability::unstable:
            return "unstable";

        case Stability::conditionally_stable:
            return "conditionally_stable";

        case Stability::stable:
            return "stable";
    }

    return "unknown";
}


inline RuntimeServiceCapabilityTraceDistributionExplanationReportSerialization
SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportView(
    const RuntimeServiceCapabilityTraceDistributionExplanationReportView&
        report)
{
    RuntimeServiceCapabilityTraceDistributionExplanationReportSerialization
        serialization;

    serialization.matched_set_stable =
        report.matched_set_stable;

    if (!report.report_defined)
    {
        return serialization;
    }

    const auto boolean_name =
        [](const bool value) noexcept
        {
            return value ? "true" : "false";
        };

    std::string payload;

    payload.reserve(768U);

    payload += "{\"schema_version\":1";

    payload += ",\"total_traces\":";
    payload += std::to_string(report.total_traces);

    payload += ",\"represented_statuses\":";
    payload += std::to_string(report.represented_statuses);

    payload += ",\"shape\":\"";
    payload +=
        RuntimeServiceCapabilityTraceDistributionShapeName(
            report.shape);
    payload += '"';

    payload += ",\"governing_rule\":\"";
    payload +=
        RuntimeServiceCapabilityTraceDistributionShapeRuleName(
            report.governing_rule);
    payload += '"';

    payload += ",\"confidence\":\"";
    payload +=
        RuntimeServiceCapabilityTraceDistributionShapeConfidenceName(
            report.confidence);
    payload += '"';

    payload += ",\"stability\":\"";
    payload +=
        RuntimeServiceCapabilityTraceDistributionExplanationStabilityName(
            report.stability);
    payload += '"';

    payload += ",\"confidence_basis_points\":";
    payload +=
        std::to_string(
            report.confidence_basis_points);

    payload += ",\"evidence_margin_basis_points\":";
    payload +=
        std::to_string(
            report.evidence_margin_basis_points);

    payload += ",\"stability_basis_points\":";
    payload +=
        std::to_string(
            report.stability_basis_points);

    payload += ",\"conflicting_evidence_count\":";
    payload +=
        std::to_string(
            report.conflicting_evidence_count);

    payload += ",\"stable\":";
    payload += boolean_name(report.stable);

    payload += ",\"conditionally_stable\":";
    payload +=
        boolean_name(
            report.conditionally_stable);

    payload += ",\"unstable\":";
    payload += boolean_name(report.unstable);

    payload += ",\"structurally_anchored\":";
    payload +=
        boolean_name(
            report.structurally_anchored);

    payload += ",\"threshold_sensitive\":";
    payload +=
        boolean_name(
            report.threshold_sensitive);

    payload += ",\"evidence_sensitive\":";
    payload +=
        boolean_name(
            report.evidence_sensitive);

    payload += ",\"sample_sensitive\":";
    payload +=
        boolean_name(
            report.sample_sensitive);

    payload += ",\"fallback_sensitive\":";
    payload +=
        boolean_name(
            report.fallback_sensitive);

    payload += ",\"matched_set_stable\":";
    payload +=
        boolean_name(
            report.matched_set_stable);

    payload += '}';

    serialization.payload =
        std::move(payload);

    serialization.serialization_available = true;

    return serialization;
}


inline RuntimeServiceCapabilityTraceDistributionExplanationReportSerialization
SerializeRuntimeServiceCapabilityTraceDistributionExplanationReport(
    const RuntimeServiceCapabilityTraceQuerySummary& summary)
{
    return
        SerializeRuntimeServiceCapabilityTraceDistributionExplanationReportView(
            BuildRuntimeServiceCapabilityTraceDistributionExplanationReportView(
                summary));
}


} // namespace sovereign
