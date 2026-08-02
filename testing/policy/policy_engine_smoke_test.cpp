#include "policy_engine.hpp"
#include "test_require.hpp"
#include <iostream>

using namespace sovereign::policy;

int main()
{
    PolicyEngine engine;
    sovereign::PrincipalId id{};
    id.bytes[0] = 1;

    auto owner = engine.Evaluate(id, {"owner"}, "configure");
    REQUIRE(owner.Ok());
    REQUIRE(owner.value.effect == PolicyEffect::Allow);

    auto ro = engine.Evaluate(id, {"readonly"}, "deploy");
    REQUIRE(ro.Ok());
    REQUIRE(ro.value.effect == PolicyEffect::Deny);

    auto op = engine.Evaluate(id, {"operator"}, "deploy");
    REQUIRE(op.Ok());
    REQUIRE(op.value.effect == PolicyEffect::Deny);

    auto op_ok = engine.Evaluate(id, {"operator"}, "read");
    REQUIRE(op_ok.Ok());
    REQUIRE(op_ok.value.effect == PolicyEffect::Allow);

    auto none = engine.Evaluate(id, {}, "read");
    REQUIRE(none.Ok());
    REQUIRE(none.value.effect == PolicyEffect::Deny);

    std::cout << "policy_engine_smoke_test: OK\n";
    return 0;
}
