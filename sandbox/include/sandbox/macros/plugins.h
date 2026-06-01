#pragma once

#include "sandbox/utilities/events.h"
#include "sandbox/events/plugins.h"

// Standard loader: Assumes "SandboxLibraryMain" as the C-linkage entry point
#define SANDBOX_PLUGIN_LOAD(ecs_ref, virtual_path) \
sandbox::events::publish(ecs_ref, sandbox::events::plugins::load_request{virtual_path})

// Custom loader: Allows specifying a custom module entry point function name
#define SANDBOX_PLUGIN_LOAD_CUSTOM(ecs_ref, virtual_path, entry_point_name) \
sandbox::events::publish(ecs_ref, sandbox::events::plugins::load_request{virtual_path, entry_point_name})

