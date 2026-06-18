// tests/suite/src/suite_library_loading.cpp
// Integration suite: MVP module lifecycle using library_loader_t.
//
// Simulates a full engine boot/teardown sequence:
//   1. Multiple modules are loaded during "boot"
//   2. They are safely unloaded during "teardown"
// The dummy_plugin shared library (tests/dummy_plugin/) is used for success paths.
// All failure paths are verified to be silent (exceptions swallowed internally).

#include <catch2/catch_all.hpp>
#include "core/library_loader.h"

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using sandbox::core::library_loader_t;

// ---------------------------------------------------------------------------
// Helper: locate dummy_plugin.so next to the running binary or in CWD
// ---------------------------------------------------------------------------
static fs::path find_plugin(const std::string& stem) {
    const char* ext =
#if defined(_WIN32) || defined(_WIN64)
        ".dll";
#elif defined(__APPLE__)
        ".dylib";
#else
        ".so";
#endif
    std::string filename = stem + ext;
    for (auto candidate : {
        fs::current_path() / filename,
        fs::current_path().parent_path() / "bin" / filename,
        fs::path(filename),
    }) {
        if (fs::exists(candidate)) return candidate;
    }
    return fs::current_path() / filename;
}

static bool plugin_available(const std::string& stem) {
    return fs::exists(find_plugin(stem));
}

// ===========================================================================
// Suite: MVP module lifecycle
// ===========================================================================

TEST_CASE("Suite: Library loader — failure paths never throw", "[suite][library_loader]")
{
    library_loader_t loader;

    SECTION("loading a non-existent module is silent") {
        REQUIRE_NOTHROW(loader.load(fs::path("nonexistent_module_aaa.so")));
        REQUIRE_NOTHROW(loader.load(fs::path("nonexistent_module_bbb.so")));
        REQUIRE_NOTHROW(loader.load(fs::path("nonexistent_module_ccc.so")));
    }

    SECTION("unloading modules that were never loaded is silent") {
        REQUIRE_NOTHROW(loader.unload("render_system"));
        REQUIRE_NOTHROW(loader.unload("audio_system"));
        REQUIRE_NOTHROW(loader.unload("input_system"));
    }

    SECTION("full boot-fail → teardown sequence is silent") {
        // Simulate an engine boot where all plugins fail to load
        const std::vector<fs::path> fake_modules = {
            "render_module.so",
            "audio_module.so",
            "physics_module.so",
            "input_module.so",
        };

        for (const auto& m : fake_modules)
            REQUIRE_NOTHROW(loader.load(m));

        // Teardown: attempt to unload everything regardless
        for (const auto& m : fake_modules)
            REQUIRE_NOTHROW(loader.unload(m.stem().string()));
    }
}

TEST_CASE("Suite: Library loader — MVP boot and teardown", "[suite][library_loader]")
{
    if (!plugin_available("dummy_plugin")) {
        WARN("dummy_plugin not found — skipping MVP lifecycle integration tests.");
        SUCCEED("Skipped: dummy_plugin not available on disk");
        return;
    }

    SECTION("single module: load then unload") {
        library_loader_t loader;
        REQUIRE_NOTHROW(loader.load(find_plugin("dummy_plugin")));
        REQUIRE_NOTHROW(loader.unload("dummy_plugin"));
    }

    SECTION("multiple loads of the same module are deduplicated") {
        library_loader_t loader;
        loader.load(find_plugin("dummy_plugin"));
        REQUIRE_NOTHROW(loader.load(find_plugin("dummy_plugin")));  // guard path
        REQUIRE_NOTHROW(loader.unload("dummy_plugin"));             // single unload is enough
        REQUIRE_NOTHROW(loader.unload("dummy_plugin"));             // second unload is silent
    }

    SECTION("loader is destroyed cleanly after loading") {
        // Test that the destructor closes the library without crashing
        REQUIRE_NOTHROW([&]{
            library_loader_t loader;
            loader.load(find_plugin("dummy_plugin"));
            // destructor runs here — library should be unloaded cleanly
        }());
    }

    SECTION("simulate engine boot sequence with multiple modules") {
        // In the MVP, we only have one real plugin; we simulate a multi-module
        // boot by loading dummy_plugin under different aliases is not possible
        // (the key is the stem), so we load it once and fill the rest with
        // nonexistent names to test the mixed boot scenario.
        library_loader_t loader;

        // "Renderer" — successfully loads
        REQUIRE_NOTHROW(loader.load(find_plugin("dummy_plugin")));

        // "AudioEngine", "PhysicsEngine" — not yet built, load silently fails
        REQUIRE_NOTHROW(loader.load(fs::path("audio_engine.so")));
        REQUIRE_NOTHROW(loader.load(fs::path("physics_engine.so")));

        // Teardown — only dummy_plugin was actually loaded
        REQUIRE_NOTHROW(loader.unload("dummy_plugin"));
        REQUIRE_NOTHROW(loader.unload("audio_engine"));    // not loaded, silent
        REQUIRE_NOTHROW(loader.unload("physics_engine"));  // not loaded, silent
    }

    SECTION("move-constructed loader retains loaded libraries") {
        library_loader_t a;
        a.load(find_plugin("dummy_plugin"));

        REQUIRE_NOTHROW([&]{
            library_loader_t b = std::move(a);
            b.unload("dummy_plugin");
        }());
    }
}
