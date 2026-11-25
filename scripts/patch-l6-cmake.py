#!/usr/bin/env python3
"""Wire node_capability_binding into capability lib + smoke test."""
from pathlib import Path
import re
import sys

root = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
cm = root / "core" / "capability" / "CMakeLists.txt"
if cm.exists():
    t = cm.read_text()
    if "node_capability_binding.cpp" not in t:
        # append to sources if pattern known
        if "capability_registry.cpp" in t:
            t = t.replace(
                "capability_registry.cpp",
                "capability_registry.cpp\n  node_capability_binding.cpp",
            )
        elif "add_library" in t:
            t += "\n# L6\ntarget_sources(sovereign_capability PRIVATE node_capability_binding.cpp)\n"
        else:
            t += "\nset(SOV_CAP_L6_SOURCES node_capability_binding.cpp)\n"
        cm.write_text(t)
        print("patched", cm)
    else:
        print("cmake capability already has binding")
else:
    print("WARN: no core/capability/CMakeLists.txt")

# testing CMake
tc = root / "testing" / "CMakeLists.txt"
if tc.exists():
    t = tc.read_text()
    if "node_capability_binding_smoke_test" not in t:
        snippet = """
# L6
if(TARGET sovereign_capability)
  add_executable(node_capability_binding_smoke_test
    capability/node_capability_binding_smoke_test.cpp)
  target_link_libraries(node_capability_binding_smoke_test PRIVATE sovereign_capability)
  target_include_directories(node_capability_binding_smoke_test PRIVATE
    ${CMAKE_SOURCE_DIR}/core/capability)
  add_test(NAME node_capability_binding_smoke_test COMMAND node_capability_binding_smoke_test)
endif()
"""
        tc.write_text(t + snippet)
        print("patched", tc)
    else:
        print("test already registered")
else:
    print("WARN: no testing/CMakeLists.txt")
