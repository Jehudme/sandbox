#include "cli/cli_parser.h"
#include "../../sandbox/include/sandbox/abi/engine.h"
#include <iostream>

int main(int argc, char* argv[]) {
    sandbox_handle_t props = sandbox::launcher::parse_cli(argc, argv);
    if (SANDBOX_HANDLE_IS_INVALID(props)) {
        return 1; // CLI parsing failed or requested exit (e.g. --help)
    }

    sandbox_engine_t* engine = sandbox_engine_create();
    
    if (!sandbox_engine_initialize(engine, props)) {
        std::cerr << "Failed to initialize engine.\n";
        sandbox_properties_destroy(&props);
        sandbox_engine_destroy(engine);
        return 1;
    }
    
    std::cout << "Engine initialized successfully!\n";
    
    sandbox_properties_destroy(&props);
    sandbox_engine_destroy(engine);
    
    return 0;
}
