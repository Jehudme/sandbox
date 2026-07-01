#include <catch2/catch_all.hpp>
#include "../../../launcher/source/cli/cli_parser.h"

TEST_CASE("CLI Parser Suite: Application Mounting", "[cli_parser][suite]") {
    SECTION("Parses application path correctly") {
        std::vector<const char*> args = {"sandbox_launcher", "my_app"};
        
        auto props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE(props.has_value());

        auto physical = props->get<std::string>("filesystem/mounts/app/physical");
        REQUIRE(physical.has_value());
        REQUIRE(physical.value() == "my_app");
        
        auto readonly = props->get<bool>("filesystem/mounts/app/readonly");
        REQUIRE(readonly.has_value());
        REQUIRE(readonly.value() == true);
    }
    
    SECTION("Parses dev flag correctly") {
        std::vector<const char*> args = {"sandbox_launcher", "my_app", "--dev"};
        
        auto props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE(props.has_value());

        auto physical = props->get<std::string>("filesystem/mounts/app/physical");
        REQUIRE(physical.has_value());
        REQUIRE(physical.value() == "my_app");
        
        auto readonly = props->get<bool>("filesystem/mounts/app/readonly");
        REQUIRE(readonly.has_value());
        REQUIRE(readonly.value() == false);
    }

    SECTION("Parses logs flag correctly") {
        std::vector<const char*> args = {"sandbox_launcher", "my_app", "--logs", "trace"};
        
        auto props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE(props.has_value());

        auto logs = props->get<std::string>("logs/level");
        REQUIRE(logs.has_value());
        REQUIRE(logs.value() == "trace");
    }
    
    SECTION("Fails without app path") {
        std::vector<const char*> args = {"sandbox_launcher"};
        auto props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE_FALSE(props.has_value());
    }
}
