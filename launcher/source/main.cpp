#include <iostream>
#include <flecs.h>

#include "sandbox/core/engine.h"
#include "sandbox/core/plugin.h"
#include "sandbox/macros/runner.h"

int main() {
    sandbox::configure_plugin_os_api();

    sandbox::engine engine;
    sandbox::properties manifest;

    engine.initialize(manifest);

    SANDBOX_RUNNER_RUN(engine.ecs);

}
