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
        
        sandbox_handle_t props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE(!(!SANDBOX_HANDLE_IS_VALID(props)));



        // Let's use a struct to be perfectly safe across C API
        struct flags { bool l; bool m; } f = {false, false};
        sandbox_properties_keys(props, "engine", [](const char* k, void* ctx) {
            auto* p = static_cast<flags*>(ctx);
            if (std::string(k) == "libraries") p->l = true;
            if (std::string(k) == "sandbox") p->m = true;
        }, &f);

        REQUIRE(f.l == true);
        REQUIRE(f.m == true);
        
        sandbox_properties_destroy(&props);
    }
    
    SECTION("Loads dummy project 2 correctly and merges with CLI args") {
        std::string config_arg = p2.string();
        std::vector<const char*> args = {"sandbox_launcher", "--config", config_arg.c_str(), "-l", "cli_plugin.so"};
        
        sandbox_handle_t props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE(!(!SANDBOX_HANDLE_IS_VALID(props)));

        struct flags { bool l; bool m; } f = {false, false};
        sandbox_properties_keys(props, "engine", [](const char* k, void* ctx) {
            auto* p = static_cast<flags*>(ctx);
            if (std::string(k) == "libraries") p->l = true;
            if (std::string(k) == "sandbox") p->m = true;
        }, &f);

        REQUIRE(f.l == true);
        REQUIRE(f.m == true);
        
        sandbox_properties_destroy(&props);
    }

    SECTION("Parses CLI arguments directly without config") {
        std::vector<const char*> args = {"sandbox_launcher", "-l", "dummy.so", "-m", "test-mod@1.0.0"};
        
        sandbox_handle_t props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
        REQUIRE(!(!SANDBOX_HANDLE_IS_VALID(props)));

        struct flags { bool l; bool m; } f = {false, false};
        sandbox_properties_keys(props, "engine", [](const char* k, void* ctx) {
            auto* p = static_cast<flags*>(ctx);
            if (std::string(k) == "libraries") p->l = true;
            if (std::string(k) == "sandbox") p->m = true;
        }, &f);

        REQUIRE(f.l == true);
        REQUIRE(f.m == true);
        
        sandbox_properties_destroy(&props);
    }

    fs::remove(p1);
    fs::remove(p2);
}
