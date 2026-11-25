#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "capability_registry.hpp"
#include "capability_types.hpp"

namespace sovereign::capability {

struct BindingResult {
  bool ok{false};
  std::string error;
};

/** Persist node_id -> capability_id bindings under data_root/capability/bindings/ */
class NodeCapabilityBinding {
 public:
  explicit NodeCapabilityBinding(std::filesystem::path data_root);

  BindingResult Bind(const std::string& node_id, const std::string& capability_id,
                     const CapabilityRegistry& registry, bool require_enabled = true);
  BindingResult Unbind(const std::string& node_id, const std::string& capability_id);
  std::vector<std::string> List(const std::string& node_id) const;

 private:
  std::filesystem::path root_;
  std::filesystem::path path_for(const std::string& node_id) const;
};

}  // namespace sovereign::capability
