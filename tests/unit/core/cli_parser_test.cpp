#include <catch2/catch_test_macros.hpp>
#include "sandbox/utilities/cli_parser.h"
#include <stdexcept>

TEST_CASE("CLI Parser", "[cli]") {
    SECTION("Format B (Unversioned)") {
        auto req = sandbox::utilities::parse_activation_argument("module");
        REQUIRE(req.module_name == "module");
        REQUIRE(req.major == 0);
        REQUIRE(req.minor == 0);
        REQUIRE(req.patch == 0);
    }
    
    SECTION("Format A (Versioned: major.minor)") {
        auto req = sandbox::utilities::parse_activation_argument("module:1.2");
        REQUIRE(req.module_name == "module");
        REQUIRE(req.major == 1);
        REQUIRE(req.minor == 2);
        REQUIRE(req.patch == 0);
    }
    
    SECTION("Format A (Versioned: major.minor.patch)") {
        auto req = sandbox::utilities::parse_activation_argument("module:1.2.3");
        REQUIRE(req.module_name == "module");
        REQUIRE(req.major == 1);
        REQUIRE(req.minor == 2);
        REQUIRE(req.patch == 3);
    }

    SECTION("Invalid strings") {
        REQUIRE_THROWS_AS(sandbox::utilities::parse_activation_argument("module:1"), std::invalid_argument);
        REQUIRE_THROWS_AS(sandbox::utilities::parse_activation_argument("module:a.b"), std::invalid_argument);
        REQUIRE_THROWS_AS(sandbox::utilities::parse_activation_argument("module:1.2.3.4"), std::invalid_argument);
        REQUIRE_THROWS_AS(sandbox::utilities::parse_activation_argument("module:1.X"), std::invalid_argument);
    }
}
