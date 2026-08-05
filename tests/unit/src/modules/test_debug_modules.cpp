#include <catch2/catch_test_macros.hpp>
#include "core/engine.h"
#include "../../test_accessor.h"
using namespace sandbox::core;

TEST_CASE("Debug Modules", "[debug]") {
    flecs::world w;
    bootstrapper_t::load_library(w, "./cmake-build-debug/bin/sandbox_plugin.so");
    
    // How to access m_modules? We can't because it's private.
    // Let's just try to activate it and catch the error.
    try {
        bootstrapper_t b;
        b.activate(w, "sandbox-configuration@1.0.0");
        FAIL("Activated successfully!");
    } catch (std::exception& e) {
        WARN("Failed to activate: " << e.what());
    }
}
