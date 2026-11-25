#include "capability_registry.hpp"
#include "node_capability_binding.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#define REQUIRE(cond)                                                          \
  do {                                                                         \
    if (!(cond)) {                                                             \
      std::cerr << "REQUIRE failed: " #cond "\n";                              \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int main() {
  namespace fs = std::filesystem;
  const fs::path root =
      fs::temp_directory_path() /
      ("sov-l6-" + std::to_string(static_cast<unsigned long>(std::rand())));
  fs::create_directories(root);

  sovereign::capability::CapabilityRegistry reg;
  REQUIRE(reg.Open(root) == sovereign::capability::CapabilityError::Ok);
  auto seed = reg.SeedDefaults();
  REQUIRE(seed.Ok());

  sovereign::capability::NodeCapabilityBinding bind(root);

  auto bad = bind.Bind("node-a", "does.not.exist", reg, true);
  REQUIRE(!bad.ok);
  REQUIRE(bad.error.find("unknown") != std::string::npos);

  auto list_res = reg.List();
  REQUIRE(list_res.Ok());
  REQUIRE(!list_res.value.empty());
  const std::string cid = list_res.value.front().id;

  auto ok = bind.Bind("node-a", cid, reg, false);
  REQUIRE(ok.ok);
  auto bound = bind.List("node-a");
  REQUIRE(std::find(bound.begin(), bound.end(), cid) != bound.end());

  auto ub = bind.Unbind("node-a", cid);
  REQUIRE(ub.ok);
  bound = bind.List("node-a");
  REQUIRE(std::find(bound.begin(), bound.end(), cid) == bound.end());

  reg.Close();
  fs::remove_all(root);
  std::cout << "node_capability_binding_smoke_test: OK\n";
  return 0;
}
