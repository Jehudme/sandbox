#include <sandbox/sandbox.h>
#include <iostream>

// 1. Define a dummy class to test the Registry
namespace sandbox {
    class test_component {
    public:
        test_component() {
            // This will use the "Active" logger from the scope below!
            SANDBOX_LOG_INFO("test_component: Constructor called.");
        }
        virtual ~test_component() = default;

        void do_something() {
            SANDBOX_LOG_DEBUG("test_component: Doing some work...");
        }
    };
}

// 2. Register the class so the Registry can find it by string
// Usually, you put this in the .cpp of your component
#include "../../sandbox/include/sandbox/reflections/registration.h"
SANDBOX_REGISTER_CLASS(sandbox::test_component, "test_component")

int main()
{
    // --- STEP 1: Setup Global Logger ---
    sandbox::logger engine_logger("Harmony", sandbox::logger::level::trace, false);

    // --- STEP 2: Enter a Scoped Context ---
    // All logs inside this { } block will now use 'engine_logger' automatically
    SANDBOX_LOGGER_SCOPE(engine_logger);

    SANDBOX_LOG_INFO("=== Starting Engine Core Test ===");

    // --- STEP 3: Test Properties (JSON Tree) ---
    sandbox::properties config;
    config.set({"engine", "version"}, "0.1.0-alpha");
    config.set({"window", "resolution", "width"}, 1920);
    config.set({"window", "resolution", "height"}, 1080);
    config.set({"window", "fullscreen"}, true);

    SANDBOX_LOG_INFO("Configuration loaded:");
    std::cout << config.show() << std::endl;

    // --- STEP 4: Test Registry (RTTR Factory) ---
    SANDBOX_LOG_INFO("Attempting to spawn 'test_component' via Registry...");

    auto my_comp = sandbox::registry::create_type<sandbox::test_component>("test_component");

    if (my_comp)
    {
        my_comp->do_something();
        SANDBOX_LOG_INFO("Registry test successful!");
    }
    else
    {
        SANDBOX_LOG_ERROR("Registry test failed! Could not create component.");
    }

    // --- STEP 5: Test Error Handling ---
    SANDBOX_LOG_WARN("Testing an intentional registry failure...");
    auto ghost = sandbox::registry::create_type<sandbox::test_component>("non_existent_type");

    SANDBOX_LOG_INFO("=== Core Test Complete ===");

    return 0;
}