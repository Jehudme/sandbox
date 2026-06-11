#include <catch2/catch_test_macros.hpp>

// Simulate a plugin build by only including SDK headers
#include "sandbox/core/plugin.h"
#include <sandbox/modules/logger/ilogger.h>
#include "sandbox/core/module_info.h"

// Instantiate mock module
class mock_plugin {
public:
    mock_plugin(flecs::world& ecs) {}
};

// Register service
SANDBOX_DECLARE_SERVICE(mock_service, 1, 0);

// Register module
SANDBOX_DECLARE_MODULE(mock_plugin, mock_plugin_impl, 1, 0, 0, "mock_service");

TEST_CASE("Contract Separation Validation", "[architecture]") {
    SECTION("Instantiate a mock module using only SDK headers") {
        auto& registry = sandbox::get_local_registry();
        
        bool found_service = false;
        for (const auto& svc : registry.services) {
            if (svc.name == "mock_service" && svc.version_major == 1 && svc.version_minor == 0) {
                found_service = true;
                break;
            }
        }
        REQUIRE(found_service);

        bool found_module = false;
        for (const auto& mod : registry.modules) {
            if (mod.name == "mock_plugin_impl" && mod.version_major == 1 && mod.version_minor == 0 && mod.version_patch == 0 && mod.provides_service == "mock_service") {
                found_module = true;
                break;
            }
        }
        REQUIRE(found_module);
    }
}
