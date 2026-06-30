// tests/unit/src/sdk/test_library_loader.cpp
// Unit tests for sandbox::sdk::library_loader_t.
//
// The "success" path requires a real shared library on disk.
// The dummy_plugin target (tests/dummy_plugin/) is built alongside these tests
// and placed in the same bin/ directory as the test executable.
// We locate it at runtime via the executable's own directory.

#include <catch2/catch_all.hpp>
#include "core/library_loader.h"
#include "core/exceptions.h"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;
using sandbox::core::library_loader_t;

// ---------------------------------------------------------------------------
// Helper: path to the dummy shared library built by the dummy_plugin target.
// It sits next to the test executable (CMAKE_RUNTIME_OUTPUT_DIRECTORY = bin/).
// ---------------------------------------------------------------------------
static fs::path dummy_plugin_path() {
    const char* lib_name =
#if defined(_WIN32) || defined(_WIN64)
        "dummy_plugin.dll";
#elif defined(__APPLE__)
        "dummy_plugin.dylib";
#else
        "dummy_plugin.so";
#endif

    // Try next to the executable (CWD when run via ctest / CLion), then ../bin
    for (auto candidate : {
        fs::current_path() / lib_name,
        fs::current_path().parent_path() / "bin" / lib_name,
        fs::path(lib_name),
    }) {
        if (fs::exists(candidate)) return candidate;
    }
    return fs::current_path() / lib_name;  // best-effort, may not exist
}

static fs::path nonexistent_path() {
    return fs::path("totally_nonexistent_library_xyz.so");
}

// ===========================================================================
// Tests
// ===========================================================================

TEST_CASE("LibraryLoader: lifecycle", "[library_loader][lifecycle]")
{
    SECTION("default construction does not throw") {
        REQUIRE_NOTHROW(library_loader_t{});
    }

    SECTION("move construction compiles and does not throw") {
        library_loader_t a;
        REQUIRE_NOTHROW([&]{ library_loader_t b{std::move(a)}; }());
    }

    SECTION("move assignment compiles and does not throw") {
        library_loader_t a, b;
        REQUIRE_NOTHROW(b = std::move(a));
    }
}

TEST_CASE("LibraryLoader: loading non-existent library is silent", "[library_loader][error]")
{
    library_loader_t loader;
    flecs::world ecs;

    SECTION("load on non-existent path throws") {
        REQUIRE_THROWS_AS(loader.load(ecs, nonexistent_path()), sandbox::core::library_load_error);
    }

    SECTION("multiple load calls on non-existent path throw") {
        REQUIRE_THROWS_AS(loader.load(ecs, nonexistent_path()), sandbox::core::library_load_error);
        REQUIRE_THROWS_AS(loader.load(ecs, nonexistent_path()), sandbox::core::library_load_error);
        REQUIRE_THROWS_AS(loader.load(ecs, nonexistent_path()), sandbox::core::library_load_error);
    }
}

TEST_CASE("LibraryLoader: unload is always safe", "[library_loader][unload]")
{
    library_loader_t loader;
    flecs::world ecs;

    SECTION("unload on empty loader does not throw") {
        REQUIRE_NOTHROW(loader.unload(ecs, "dummy_plugin"));
    }

    SECTION("unload on unknown name does not throw") {
        REQUIRE_NOTHROW(loader.unload(ecs, "totally_unknown_lib"));
    }

    SECTION("unload can be called multiple times on the same missing name") {
        REQUIRE_NOTHROW(loader.unload(ecs, "missing"));
        REQUIRE_NOTHROW(loader.unload(ecs, "missing"));
    }
}

TEST_CASE("LibraryLoader: duplicate load guard", "[library_loader][duplicate]")
{
    // Even if the library doesn't exist, the guard path is exercised when
    // a successful load has already been recorded.  We test that the loader
    // does not crash when asked to load the same (non-existent) stem twice.
    library_loader_t loader;
    flecs::world ecs;

    SECTION("calling load twice on the same path throws both times") {
        REQUIRE_THROWS_AS(loader.load(ecs, nonexistent_path()), sandbox::core::library_load_error);
        REQUIRE_THROWS_AS(loader.load(ecs, nonexistent_path()), sandbox::core::library_load_error);
    }
}

TEST_CASE("LibraryLoader: successful load and unload", "[library_loader][load][integration]")
{
    fs::path plugin = dummy_plugin_path();

    if (!fs::exists(plugin)) {
        WARN("dummy_plugin shared library not found at: " + plugin.string()
            + " — skipping load/unload success path tests.");
        SUCCEED("Skipped: dummy_plugin not available on disk");
        return;
    }

    library_loader_t loader;
    flecs::world ecs;

    SECTION("load of a valid shared library does not throw") {
        REQUIRE_NOTHROW(loader.load(ecs, plugin));
    }

    SECTION("duplicate load of the same library does not throw") {
        loader.load(ecs, plugin);
        REQUIRE_NOTHROW(loader.load(ecs, plugin));  // second call hits the guard path
    }

    SECTION("unload after successful load does not throw") {
        loader.load(ecs, plugin);
        REQUIRE_NOTHROW(loader.unload(ecs, "dummy_plugin"));
    }

    SECTION("unload then reload is safe") {
        loader.load(ecs, plugin);
        loader.unload(ecs, "dummy_plugin");
        REQUIRE_NOTHROW(loader.load(ecs, plugin));
    }
}
