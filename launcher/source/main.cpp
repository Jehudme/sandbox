#include "cli/cli_parser.h"
#include <sandbox/sdk/engine.hpp>
#include <iostream>
#include <exception>

int main(int argc, char* argv[]) {
    try {
        std::optional<sandbox::properties> props = sandbox::launcher::parse_cli(argc, argv);
        
        // If parsing failed or exit requested (e.g., --help), props will be nullopt
        if (!props) {
            return 1;
        }

        sandbox::engine engine;
        
        if (!engine.initialize(*props)) {
            std::cerr << "[Launcher] Failed to initialize engine.\n";
            return 1;
        }
        
        std::cout << "[Launcher] Engine initialized successfully!\n";
        
        // The engine and its properties will be safely cleaned up 
        // when the objects go out of scope.
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "[Launcher] Fatal error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "[Launcher] Fatal error: Unknown exception occurred.\n";
        return 1;
    }
}
