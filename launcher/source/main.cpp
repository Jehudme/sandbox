#include <iostream>
#include <flecs.h>

#include "sandbox/core/engine.h"
#include "sandbox/core/plugin.h"

int main() {
    sandbox::configure_plugin_os_api();

    sandbox::engine engine;
    sandbox::properties manifest;

    engine.initialize(manifest);
}
