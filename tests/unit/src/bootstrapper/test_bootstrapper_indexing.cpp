#include <catch2/catch_all.hpp>
#include "../../test_accessor.h"
#include "core/exceptions.h"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;
using sandbox::core::bootstrapper_t;

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

TEST_CASE("Bootstrapper: indexing libraries", "[bootstrapper][indexing]") {
    bootstrapper_test_accessor::reset();

    flecs::world ecs;
    SECTION("load_library with valid path does not throw") {
        fs::path plugin = dummy_plugin_path();
        if (fs::exists(plugin)) {
            REQUIRE_NOTHROW(bootstrapper_t::load_library(ecs, plugin));
        } else {
            SUCCEED("Skipped: dummy_plugin not available on disk");
        }
    }

    SECTION("load_library with invalid path throws library_load_error") {
        REQUIRE_THROWS_AS(bootstrapper_t::load_library(ecs, "nonexistent_lib.so"), sandbox::core::library_load_error);
    }
    
    bootstrapper_test_accessor::reset();
}
