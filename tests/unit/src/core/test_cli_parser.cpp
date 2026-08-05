#include <catch2/catch_all.hpp>
#include "../../../launcher/source/cli/cli_parser.h"

TEST_CASE("CLI Parser Suite: Application Mounting", "[cli_parser][suite]") {
    SECTION("Parses application path correctly") {
        std::vector<const char*> args = {"sandbox_launcher", "my_app"};
        
        auto props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE(props.has_value());

        auto physical = props->get<std::string>("booting-configuration/mount-path");
        REQUIRE(physical.has_value());
        REQUIRE(physical.value() == "my_app");
    }
    
    SECTION("Parses dev flag correctly") {
        std::vector<const char*> args = {"sandbox_launcher", "my_app", "--dev"};
        
        auto props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE(props.has_value());

        auto physical = props->get<std::string>("booting-configuration/mount-path");
        REQUIRE(physical.has_value());
        REQUIRE(physical.value() == "my_app");
    }

    SECTION("Parses logs flag correctly") {
        std::vector<const char*> args = {"sandbox_launcher", "my_app", "--logs-level", "trace", "--logs-console", "--logs-file-max-size", "1024"};
        
        auto props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE(props.has_value());

        auto logs_level = props->get<std::string>("booting-configuration/logs-level");
        REQUIRE(logs_level.has_value());
        REQUIRE(logs_level.value() == "trace");

        auto console_enabled = props->get<bool>("booting-configuration/logs-console_enabled");
        REQUIRE(console_enabled.has_value());
        REQUIRE(console_enabled.value() == true);

        auto file_max_size = props->get<int64_t>("booting-configuration/logs-file_max_size");
        REQUIRE(file_max_size.has_value());
        REQUIRE(file_max_size.value() == 1024);
    }
    
    SECTION("Fails without app path") {
        std::vector<const char*> args = {"sandbox_launcher"};
        auto props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE_FALSE(props.has_value());
    }
}
