#include "runtime_service_capability_trace_query.hpp"

#include <cstddef>
#include <cstdint>
#include <deque>
#include "test_require.hpp"

namespace
{

sovereign::RuntimeServiceCapabilityPipelineTrace
MakeTrace(
    const std::uint64_t sequence,
    const char* capability)
{
    sovereign::RuntimeServiceCapabilityPipelineTrace
        trace;

    trace.invocation_sequence = sequence;
    trace.capability = capability;
    trace.operation = "execute";
    trace.completed = true;
    trace.result_status = 0U;

    return trace;
}

} // namespace

int main()
{
    using namespace sovereign;

    const std::deque<
        RuntimeServiceCapabilityPipelineTrace>
        traces{
            MakeTrace(1U, "alpha"),
            MakeTrace(2U, "beta"),
            MakeTrace(3U, "alpha"),
            MakeTrace(4U, "beta"),
            MakeTrace(5U, "alpha")};

    /*
     * Default RT-122 behavior remains unchanged.
     */
    const auto all =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            RuntimeServiceCapabilityTraceQuery{});

    REQUIRE(all.offset == 0U);
    REQUIRE(all.matched_count == 5U);
    REQUIRE(all.returned_count == 5U);
    REQUIRE(all.traces.size() == 5U);
    REQUIRE(!all.has_more);
    REQUIRE(!all.truncated);

    REQUIRE(
        all.traces[0U].invocation_sequence ==
        1U);

    REQUIRE(
        all.traces[4U].invocation_sequence ==
        5U);

    /*
     * Pagination occurs after oldest-first ordering.
     */
    RuntimeServiceCapabilityTraceQuery
        oldest_page_query;

    oldest_page_query.offset = 1U;
    oldest_page_query.maximum_results = 2U;

    const auto oldest_page =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            oldest_page_query);

    REQUIRE(oldest_page.offset == 1U);
    REQUIRE(oldest_page.matched_count == 5U);
    REQUIRE(oldest_page.returned_count == 2U);
    REQUIRE(oldest_page.traces.size() == 2U);
    REQUIRE(oldest_page.has_more);
    REQUIRE(oldest_page.truncated);

    REQUIRE(
        oldest_page.traces[0U]
            .invocation_sequence ==
        2U);

    REQUIRE(
        oldest_page.traces[1U]
            .invocation_sequence ==
        3U);

    /*
     * Pagination occurs after newest-first ordering.
     */
    RuntimeServiceCapabilityTraceQuery
        newest_page_query;

    newest_page_query.sort_order =
        RuntimeServiceCapabilityTraceSortOrder::
            NewestFirst;

    newest_page_query.offset = 1U;
    newest_page_query.maximum_results = 2U;

    const auto newest_page =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            newest_page_query);

    REQUIRE(newest_page.offset == 1U);
    REQUIRE(newest_page.matched_count == 5U);
    REQUIRE(newest_page.returned_count == 2U);
    REQUIRE(newest_page.has_more);
    REQUIRE(newest_page.truncated);

    REQUIRE(
        newest_page.traces[0U]
            .invocation_sequence ==
        4U);

    REQUIRE(
        newest_page.traces[1U]
            .invocation_sequence ==
        3U);

    /*
     * Filtering precedes offset application.
     *
     * Ordered alpha matches:
     *   1, 3, 5
     *
     * Offset one therefore selects sequence three.
     */
    RuntimeServiceCapabilityTraceQuery
        filtered_page_query;

    filtered_page_query.capability =
        "alpha";

    filtered_page_query.offset = 1U;
    filtered_page_query.maximum_results = 1U;

    const auto filtered_page =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            filtered_page_query);

    REQUIRE(filtered_page.offset == 1U);
    REQUIRE(filtered_page.matched_count == 3U);
    REQUIRE(filtered_page.returned_count == 1U);
    REQUIRE(filtered_page.traces.size() == 1U);
    REQUIRE(filtered_page.has_more);
    REQUIRE(filtered_page.truncated);

    REQUIRE(
        filtered_page.traces[0U]
            .invocation_sequence ==
        3U);

    /*
     * A final page reports no remaining matches.
     */
    RuntimeServiceCapabilityTraceQuery
        final_page_query;

    final_page_query.offset = 3U;
    final_page_query.maximum_results = 2U;

    const auto final_page =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            final_page_query);

    REQUIRE(final_page.offset == 3U);
    REQUIRE(final_page.matched_count == 5U);
    REQUIRE(final_page.returned_count == 2U);
    REQUIRE(!final_page.has_more);
    REQUIRE(final_page.truncated);

    REQUIRE(
        final_page.traces[0U]
            .invocation_sequence ==
        4U);

    REQUIRE(
        final_page.traces[1U]
            .invocation_sequence ==
        5U);

    /*
     * An offset equal to the match count produces an empty,
     * terminal page.
     */
    RuntimeServiceCapabilityTraceQuery
        terminal_query;

    terminal_query.offset = 5U;
    terminal_query.maximum_results = 2U;

    const auto terminal_page =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            terminal_query);

    REQUIRE(terminal_page.offset == 5U);
    REQUIRE(terminal_page.matched_count == 5U);
    REQUIRE(terminal_page.returned_count == 0U);
    REQUIRE(terminal_page.traces.empty());
    REQUIRE(!terminal_page.has_more);
    REQUIRE(terminal_page.truncated);

    /*
     * An offset beyond the match count is also terminal.
     */
    RuntimeServiceCapabilityTraceQuery
        beyond_query;

    beyond_query.offset = 20U;
    beyond_query.maximum_results = 2U;

    const auto beyond_page =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            beyond_query);

    REQUIRE(beyond_page.offset == 20U);
    REQUIRE(beyond_page.matched_count == 5U);
    REQUIRE(beyond_page.returned_count == 0U);
    REQUIRE(beyond_page.traces.empty());
    REQUIRE(!beyond_page.has_more);
    REQUIRE(beyond_page.truncated);

    /*
     * A zero result limit is a metadata-only query.
     */
    RuntimeServiceCapabilityTraceQuery
        metadata_query;

    metadata_query.offset = 1U;
    metadata_query.maximum_results = 0U;

    const auto metadata_page =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            metadata_query);

    REQUIRE(metadata_page.offset == 1U);
    REQUIRE(metadata_page.matched_count == 5U);
    REQUIRE(metadata_page.returned_count == 0U);
    REQUIRE(metadata_page.traces.empty());
    REQUIRE(metadata_page.has_more);
    REQUIRE(metadata_page.truncated);

    /*
     * Unlimited results beginning at a nonzero offset return
     * every remaining match.
     */
    RuntimeServiceCapabilityTraceQuery
        unlimited_tail_query;

    unlimited_tail_query.offset = 2U;

    const auto unlimited_tail =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            unlimited_tail_query);

    REQUIRE(unlimited_tail.offset == 2U);
    REQUIRE(unlimited_tail.matched_count == 5U);
    REQUIRE(unlimited_tail.returned_count == 3U);
    REQUIRE(unlimited_tail.traces.size() == 3U);
    REQUIRE(!unlimited_tail.has_more);
    REQUIRE(unlimited_tail.truncated);

    REQUIRE(
        unlimited_tail.traces[0U]
            .invocation_sequence ==
        3U);

    REQUIRE(
        unlimited_tail.traces[2U]
            .invocation_sequence ==
        5U);

    /*
     * No matches remain an untruncated empty result when no
     * offset was requested.
     */
    RuntimeServiceCapabilityTraceQuery
        no_match_query;

    no_match_query.capability =
        "missing";

    const auto no_matches =
        QueryRuntimeServiceCapabilityTraces(
            traces,
            no_match_query);

    REQUIRE(no_matches.offset == 0U);
    REQUIRE(no_matches.matched_count == 0U);
    REQUIRE(no_matches.returned_count == 0U);
    REQUIRE(no_matches.traces.empty());
    REQUIRE(!no_matches.has_more);
    REQUIRE(!no_matches.truncated);

    return 0;
}
