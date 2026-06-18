// tests/unit/src/bootstrapper/test_bootstrapper_indexing.cpp
#include <catch2/catch_all.hpp>
#include "core/bootstrapper.h"
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
    bootstrapper_t::reset();

    SECTION("index_library with valid path does not throw") {
        fs::path plugin = dummy_plugin_path();
        if (fs::exists(plugin)) {
            REQUIRE_NOTHROW(bootstrapper_t::index_library(plugin));
        } else {
            SUCCEED("Skipped: dummy_plugin not available on disk");
        }
    }

    SECTION("index_library with invalid path does not throw (caught by loader)") {
        REQUIRE_NOTHROW(bootstrapper_t::index_library("nonexistent_lib.so"));
    }

    SECTION("index_libraries_in_directory with valid directory does not throw") {
        fs::path plugin = dummy_plugin_path();
        if (fs::exists(plugin)) {
            REQUIRE_NOTHROW(bootstrapper_t::index_libraries_in_directory(plugin.parent_path()));
        } else {
            SUCCEED("Skipped: dummy_plugin not available on disk");
        }
    }

    SECTION("index_libraries_in_directory with invalid directory does not throw") {
        REQUIRE_NOTHROW(bootstrapper_t::index_libraries_in_directory("nonexistent_directory"));
    }
    
    bootstrapper_t::reset();
}
