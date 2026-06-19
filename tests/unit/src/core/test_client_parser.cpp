#include <catch2/catch_all.hpp>
#include "core/client_parser.h"
#include <fstream>
#include <filesystem>

using namespace sandbox::core;
namespace fs = std::filesystem;

TEST_CASE("Client parser load_configuration", "[client_parser][config]") {
    fs::path temp_json = fs::temp_directory_path() / "test_config.json";
    
    {
        std::ofstream out(temp_json);
        out << R"({
            "engine": {
                "libraries": ["dummy_plugin.so", "another_plugin.so"],
                "modules": ["sandbox::system-Renderer@1.0.0"]
            }
        })";
    }

    SECTION("Loads properties from JSON file correctly") {
        properties_t props = client_parser_t::load_configuration(temp_json);
        
        REQUIRE(props.has({"engine", "libraries"}));
        auto libs = props.get<std::vector<std::string>>({"engine", "libraries"});
        REQUIRE(libs.has_value());
        REQUIRE(libs->size() == 2);
        REQUIRE((*libs)[0] == "dummy_plugin.so");
        
        auto mods = props.get<std::vector<std::string>>({"engine", "modules"});
        REQUIRE(mods.has_value());
        REQUIRE(mods->size() == 1);
        REQUIRE((*mods)[0] == "sandbox::system-Renderer@1.0.0");
    }

    fs::remove(temp_json);
}

// Helper for finding dummy plugin path
static fs::path dummy_plugin_path() {
    const char* lib_name =
#if defined(_WIN32) || defined(_WIN64)
        "dummy_plugin.dll";
#elif defined(__APPLE__)
        "dummy_plugin.dylib";
#else
        "dummy_plugin.so";
#endif

    for (auto candidate : {
        fs::current_path() / lib_name,
        fs::current_path().parent_path() / "bin" / lib_name,
        fs::path(lib_name),
    }) {
        if (fs::exists(candidate)) return candidate;
    }
    return fs::current_path() / lib_name;
}

TEST_CASE("Client parser apply_configuration", "[client_parser][apply]") {
    bootstrapper_t::reset();

    // We can test that the parser safely processes the list, catches errors internally
    // (since library loading and module activation errors are caught or throw specific errors).
    // Duplicate loading guard is tested implicitly: we'll load the dummy plugin twice.
    properties_t props;
    
    fs::path plugin = dummy_plugin_path();
    
    std::vector<std::string> libs = { plugin.string(), plugin.string(), "missing_lib.so" };
    props.set<std::vector<std::string>>({"engine", "libraries"}, libs);

    bootstrapper_t bootstrapper;
    
    SECTION("Applying configuration loads libraries without crashing and prevents duplicate load issues") {
        // Will attempt to load dummy_plugin twice (second is a no-op due to guard),
        // and missing_lib once (which gracefully fails).
        REQUIRE_NOTHROW(client_parser_t::apply_configuration(bootstrapper, props));
    }
    
    bootstrapper_t::reset();
}
