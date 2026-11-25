#include "node_capability_binding.hpp"

#include <cctype>
#include <fstream>
#include <set>

namespace sovereign::capability {
namespace fs = std::filesystem;

NodeCapabilityBinding::NodeCapabilityBinding(fs::path data_root)
    : root_(std::move(data_root) / "capability" / "bindings") {
  std::error_code ec;
  fs::create_directories(root_, ec);
}

fs::path NodeCapabilityBinding::path_for(const std::string& node_id) const {
  std::string safe;
  for (char c : node_id) {
    if (std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-')
      safe.push_back(c);
    else
      safe.push_back('_');
  }
  if (safe.empty()) safe = "invalid";
  return root_ / (safe + ".txt");
}

BindingResult NodeCapabilityBinding::Bind(const std::string& node_id,
                                          const std::string& capability_id,
                                          const CapabilityRegistry& registry,
                                          bool require_enabled) {
  BindingResult r;
  if (node_id.empty() || capability_id.empty()) {
    r.error = "empty node or capability id";
    return r;
  }
  auto got = registry.Get(capability_id);
  if (!got.Ok()) {
    r.error = "unknown capability: " + capability_id;
    return r;
  }
  if (require_enabled && !got.value.enabled) {
    r.error = "capability disabled: " + capability_id;
    return r;
  }
  auto existing = List(node_id);
  std::set<std::string> set(existing.begin(), existing.end());
  set.insert(capability_id);
  std::ofstream out(path_for(node_id), std::ios::trunc);
  if (!out) {
    r.error = "write failed";
    return r;
  }
  for (const auto& id : set) out << id << "\n";
  r.ok = true;
  return r;
}

BindingResult NodeCapabilityBinding::Unbind(const std::string& node_id,
                                            const std::string& capability_id) {
  BindingResult r;
  auto existing = List(node_id);
  std::ofstream out(path_for(node_id), std::ios::trunc);
  if (!out) {
    r.error = "write failed";
    return r;
  }
  for (const auto& id : existing) {
    if (id != capability_id) out << id << "\n";
  }
  r.ok = true;
  return r;
}

std::vector<std::string> NodeCapabilityBinding::List(const std::string& node_id) const {
  std::vector<std::string> out;
  std::ifstream in(path_for(node_id));
  std::string line;
  while (std::getline(in, line)) {
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
    if (!line.empty()) out.push_back(line);
  }
  return out;
}

}  // namespace sovereign::capability
