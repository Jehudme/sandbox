#include "cli/cli_parser.h"
#include <sandbox/sdk/engine.hpp>
#include <iostream>
#include <exception>
#include "sandbox/sdk/runtime.hpp"

int main(int argc, char* argv[]) {
    try {
        std::optional<sandbox::properties> props = sandbox::launcher::parse_cli(argc, argv);
        if (!props) return 1;

        sandbox::engine engine;
        if (engine.initialize(*props)) {
            flecs::world ecs(static_cast<ecs_world_t*>(engine.get_ecs()));
            sandbox::modules::runtime::run(ecs);
            return 0;
        }

        return 1;


    } catch (const std::exception& e) {
        std::cerr << "[Launcher] Fatal error: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "[Launcher] Fatal error: Unknown exception occurred.\n";
        return 1;
    }
}
