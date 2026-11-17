#pragma once

#include "memory_types.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace sovereign::memory
{

// Blueprint-aligned memory facets (tag prefix "facet:<name>").
inline constexpr const char* kFacetConstitutional = "constitutional";
inline constexpr const char* kFacetWorking = "working";
inline constexpr const char* kFacetDecision = "decision";
inline constexpr const char* kFacetOperational = "operational";
inline constexpr const char* kFacetKnowledge = "knowledge";
inline constexpr const char* kFacetExecution = "execution";

inline constexpr const char* kAllFacets[] = {
    kFacetConstitutional, kFacetWorking, kFacetDecision,
    kFacetOperational,    kFacetKnowledge, kFacetExecution,
};

[[nodiscard]] inline std::string FacetTag(std::string_view facet)
{
    return std::string("facet:") + std::string(facet);
}

[[nodiscard]] inline bool IsKnownFacet(std::string_view facet)
{
    for (const char* f : kAllFacets)
    {
        if (facet == f)
            return true;
    }
    return false;
}

[[nodiscard]] inline std::string FacetFromTags(const std::vector<std::string>& tags)
{
    for (const auto& t : tags)
    {
        if (t.rfind("facet:", 0) == 0)
            return t.substr(6);
    }
    return {};
}

struct FacetCount
{
    std::string facet;
    std::size_t count{0};
};

} // namespace sovereign::memory
