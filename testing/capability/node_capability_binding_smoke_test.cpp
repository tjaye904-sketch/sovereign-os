#include "capability_registry.hpp"
#include "node_capability_binding.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#define REQUIRE(cond)                                                          \
  do {                                                                         \
    if (!(cond)) {                                                             \
      std::cerr << "REQUIRE failed: " #cond " at " << __FILE__ << ":"          \
                << __LINE__ << "\n";                                           \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main() {
  namespace fs = std::filesystem;
  const fs::path root =
      fs::temp_directory_path() /
      ("sov-l6-" + std::to_string(static_cast<unsigned long>(std::rand())));
  fs::create_directories(root);

  sovereign::capability::CapabilityRegistry reg(root);
  // seed one cap via registry API if available — else register manually
  auto seed = reg.SeedDefaults();
  (void)seed;

  sovereign::capability::NodeCapabilityBinding bind(root);

  auto bad = bind.Bind("node-a", "does.not.exist", reg, true);
  REQUIRE(!bad.ok);
  REQUIRE(bad.error.find("unknown") != std::string::npos);

  // pick a known id from list
  auto list = reg.List();
  REQUIRE(!list.empty());
  const std::string cid = list.front().id;

  auto ok = bind.Bind("node-a", cid, reg, false);
  REQUIRE(ok.ok);

  auto bound = bind.List("node-a");
  REQUIRE(std::find(bound.begin(), bound.end(), cid) != bound.end());

  auto ub = bind.Unbind("node-a", cid);
  REQUIRE(ub.ok);
  bound = bind.List("node-a");
  REQUIRE(std::find(bound.begin(), bound.end(), cid) == bound.end());

  fs::remove_all(root);
  std::cout << "node_capability_binding_smoke_test: OK\n";
  return 0;
}
