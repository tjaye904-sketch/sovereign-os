#pragma once
#include <string>
#include <vector>

namespace sovereign::workspace {

struct WorkspaceBlueprint {
    std::string slug, title, role;
    std::vector<std::string> capabilities, os_tools, applications, functions;
};

const std::vector<WorkspaceBlueprint>& StagedBlueprints();
const WorkspaceBlueprint* FindBlueprint(const std::string& slug);

} // namespace sovereign::workspace
