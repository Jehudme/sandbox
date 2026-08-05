#include "cli/cli_parser.h"
#include <exception>
#include <iostream>
#include <sandbox/sdk/engine.hpp>
#include <sandbox/sdk/runtime.hpp>


int main(int argc, char *argv[]) {
  try {
    std::optional<sandbox::properties> props =
        sandbox::launcher::parse_cli(argc, argv);
    if (!props)
      return 1;

    sandbox::engine engine;
    if (engine.initialize(*props)) {
      flecs::world ecs(static_cast<ecs_world_t *>(engine.get_ecs()));

      // Because plugins might provide the runtime service, the launcher's BSS
      // copy of the component ID might be 0. We need to fetch it from the world
      // by name.
      ecs_entity_t runtime_id =
          ecs_lookup(ecs.c_ptr(), "sandbox_runtime_service_t");
      if (runtime_id != 0) {
        ecs_id(sandbox_runtime_service_t) = runtime_id;
      }

      sandbox::modules::runtime::run(ecs);
      return 0;
    }

    return 1;

  } catch (const std::exception &e) {
    std::cerr << "[Launcher] Fatal error: " << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "[Launcher] Fatal error: Unknown exception occurred.\n";
    return 1;
  }
}
