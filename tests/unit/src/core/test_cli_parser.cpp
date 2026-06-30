#include <catch2/catch_all.hpp>
#include "../../../launcher/source/cli/cli_parser.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

TEST_CASE("CLI Parser Suite: Configuration Projects", "[cli_parser][config][suite]") {
    fs::path p1 = fs::temp_directory_path() / "dummy_project_1.json";
    fs::path p2 = fs::temp_directory_path() / "dummy_project_2.json";
    
    {
        std::ofstream out1(p1);
        out1 << R"({
            "engine": {
                "libraries": ["dummy_plugin.so"],
                "sandbox": ["test::sys-Renderer@1.0.0"]
            }
        })";
        
        std::ofstream out2(p2);
        out2 << R"({
            "engine": {
                "libraries": ["audio_plugin.so", "physics_plugin.so"],
                "sandbox": ["test::sys-Audio@2.0.0", "test::sys-Physics@1.0.0"]
            }
        })";
    }

    SECTION("Loads dummy project 1 correctly") {
        std::string config_arg = p1.string();
        std::vector<const char*> args = {"sandbox_launcher", "--config", config_arg.c_str()};
        
        auto props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE(props.has_value());

        auto keys = props->keys("engine");
        REQUIRE(std::find(keys.begin(), keys.end(), "libraries") != keys.end());
        REQUIRE(std::find(keys.begin(), keys.end(), "sandbox") != keys.end());
    }
    
    SECTION("Loads dummy project 2 correctly and merges with CLI args") {
        std::string config_arg = p2.string();
        std::vector<const char*> args = {"sandbox_launcher", "--config", config_arg.c_str(), "-l", "cli_plugin.so"};
        
        auto props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE(props.has_value());

        auto keys = props->keys("engine");
        REQUIRE(std::find(keys.begin(), keys.end(), "libraries") != keys.end());
        REQUIRE(std::find(keys.begin(), keys.end(), "sandbox") != keys.end());
    }

    SECTION("Parses CLI arguments directly without config") {
        std::vector<const char*> args = {"sandbox_launcher", "-l", "dummy.so", "-m", "test-mod@1.0.0"};
        
        auto props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE(props.has_value());

        auto keys = props->keys("engine");
        REQUIRE(std::find(keys.begin(), keys.end(), "libraries") != keys.end());
        REQUIRE(std::find(keys.begin(), keys.end(), "sandbox") != keys.end());
    }

    fs::remove(p1);
    fs::remove(p2);
}
