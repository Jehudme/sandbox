#include <catch2/catch_test_macros.hpp>
#include "core/engine.h"
#include "core/bootstrapper.h"
using namespace sandbox::core;

TEST_CASE("Debug Modules", "[debug]") {
    bootstrapper_t::reset();
    bootstrapper_t::index_library("./configuration.so");
    
    // How to access m_modules? We can't because it's private.
    // Let's just try to activate it and catch the error.
    try {
        bootstrapper_t b;
        b.activate("sandbox-configuration@1.0.0");
        FAIL("Activated successfully!");
    } catch (std::exception& e) {
        WARN("Failed to activate: " << e.what());
    }
}
