#pragma once

#include <string>
#include <vector>

namespace sovereign::domain
{

struct DomainProfile
{
    std::string name;           // workspace kind / domain id
    std::string system_prompt;  // model system message for this domain
    std::vector<std::string> allowed_tools;
    std::vector<std::string> denied_actions; // policy-style action strings
};

// Built-in profiles for vision domains (extensible later via files).
std::vector<DomainProfile> BuiltinProfiles();

const DomainProfile* FindProfile(const std::string& name);

// Tool allowed for domain?
bool ToolAllowed(const DomainProfile& profile, const std::string& tool_name);

// Action denied for domain (beyond global policy)?
bool ActionDenied(const DomainProfile& profile, const std::string& action);

} // namespace sovereign::domain
