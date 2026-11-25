#include "node_registry.hpp"
#include "peer_auth.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <iostream>
#include <string>

#define REQUIRE(c) do { if (!(c)) { std::cerr << "REQUIRE failed: " #c " (line " << __LINE__ << ")\n"; return 1; } } while (0)

using sovereign::federation::PeerAuth;
using sovereign::federation::PeerAuthError;

int main()
{
    namespace fs = std::filesystem;
    const auto root = fs::temp_directory_path() / "sov-l7-peer-auth";
    fs::remove_all(root);

    sovereign::node::NodeRegistry nodes;
    REQUIRE(nodes.Open(root) == sovereign::node::NodeError::Ok);
    auto peer_node = nodes.RegisterPeer("edge-1", "10.0.0.2:9000");
    REQUIRE(peer_node.Ok());
    const std::string nid = sovereign::node::NodeRegistry::IdHex(peer_node.value);

    // Known-answer vectors (FIPS 180-4 / RFC 4231 case 2).
    REQUIRE(sovereign::federation::Sha256Hex("abc") ==
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    REQUIRE(sovereign::federation::HmacSha256Hex("Jefe", "what do ya want for nothing?") ==
            "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843");

    PeerAuth auth(root);

    // Only nodes present in the node registry may enroll.
    REQUIRE(auth.Enroll("deadbeefdeadbeefdeadbeefdeadbeef", nodes).error == PeerAuthError::UnknownNode);
    REQUIRE(auth.Enroll("not-hex!", nodes).error == PeerAuthError::InvalidArgument);

    auto enrolled = auth.Enroll(nid, nodes);
    REQUIRE(enrolled.Ok());
    const std::string secret = enrolled.value;
    REQUIRE(auth.Enroll(nid, nodes).error == PeerAuthError::AlreadyEnrolled);
    REQUIRE(auth.IsEnrolled(nid));

    // Happy path.
    auto ch = auth.IssueChallenge(nid);
    REQUIRE(ch.Ok());
    REQUIRE(auth.Verify(nid, ch.value, PeerAuth::Respond(secret, nid, ch.value)).Ok());

    // Challenge is single-use (replay rejected).
    REQUIRE(auth.Verify(nid, ch.value, PeerAuth::Respond(secret, nid, ch.value)).error == PeerAuthError::BadChallenge);

    // Wrong secret rejected.
    auto ch2 = auth.IssueChallenge(nid);
    REQUIRE(auth.Verify(nid, ch2.value, PeerAuth::Respond("00", nid, ch2.value)).error == PeerAuthError::BadResponse);

    // Failed attempt consumed the challenge too.
    REQUIRE(auth.Verify(nid, ch2.value, PeerAuth::Respond(secret, nid, ch2.value)).error == PeerAuthError::BadChallenge);

    // Response bound to the node id.
    auto ch3 = auth.IssueChallenge(nid);
    REQUIRE(auth.Verify(nid, ch3.value, PeerAuth::Respond(secret, "other", ch3.value)).error == PeerAuthError::BadResponse);

    // Persistence: a new instance still verifies; raw secret is not on disk.
    PeerAuth again(root);
    REQUIRE(again.IsEnrolled(nid));
    auto ch4 = again.IssueChallenge(nid);
    REQUIRE(again.Verify(nid, ch4.value, PeerAuth::Respond(secret, nid, ch4.value)).Ok());
    {
        std::ifstream f(root / "federation" / "peers.dat");
        std::string all((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        REQUIRE(all.find(secret) == std::string::npos);
    }

    // Revocation.
    REQUIRE(again.Revoke(nid).Ok());
    REQUIRE(!again.IsEnrolled(nid));
    REQUIRE(again.IssueChallenge(nid).error == PeerAuthError::Revoked);
    REQUIRE(again.Revoke("ffff").error == PeerAuthError::NotEnrolled);

    nodes.Close();
    fs::remove_all(root);
    std::cout << "peer_auth_smoke_test: OK\n";
    return 0;
}
