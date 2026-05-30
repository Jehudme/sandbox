#include "sandbox/core/engine.h"
#include "sandbox/utilities/properties.h"
#include "sandbox/core/plugin.h"
#include <iostream>

int main(int argc, char** argv) {

    // 1. Initialize OS Plugin Support
    sandbox::configure_plugin_os_api();

    // 2. Boot the engine
    sandbox::engine engine;
    sandbox::properties manifest;

    try {
        engine.initialize(manifest);
        std::cout << "[Engine] Successfully initialized and loaded libraries!\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[Engine Crash] " << e.what() << "\n";
    }

    return 0;
}