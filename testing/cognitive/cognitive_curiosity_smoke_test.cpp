#include "curiosity_engine.hpp"
#include "test_require.hpp"
#include <filesystem>
#include <iostream>

int main()
{
    using namespace sovereign::cognitive;

    CuriosityEngine curiosity(nullptr);
    PrincipalId user{};
    user.bytes[0] = 7;

    auto proposal = curiosity.Propose(user, "battery performance", {});
    REQUIRE(proposal.Ok());
    REQUIRE(proposal.value.topic == "battery performance");
    REQUIRE(proposal.value.questions.size() >= 2);
    REQUIRE(proposal.value.questions[0].find("battery") != std::string::npos ||
            proposal.value.questions[0].find("performance") != std::string::npos);

    curiosity.QueueDiscovery("battery performance", 0.9);
    auto queue = curiosity.DiscoveryQueueSnapshot();
    REQUIRE(!queue.empty());
    REQUIRE(queue.front().topic == "battery performance");

    curiosity.ObserveInterest("battery performance", 1.5);
    auto graph = curiosity.InterestGraph(5);
    REQUIRE(!graph.empty());
    REQUIRE(graph.front().topic == "battery performance");

    std::cout << "cognitive_curiosity_smoke_test: OK\n";
    return 0;
}
