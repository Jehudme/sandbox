#pragma once

#include <filesystem>
#include <string>
#include <stdexcept>
#include "sandbox/core/platform.h"

namespace sandbox::events::plugins {

    // Plugin exception classes (dissolved from exceptions/plugins.h)
    class SANDBOX_API plugin_error : public std::runtime_error {
    public:
        plugin_error(const std::string& context, const std::filesystem::path& path, const std::string& details)
            : std::runtime_error("[" + context + "] " + path.generic_string() + ": " + details) {}
    };

    class plugin_load_error : public plugin_error {
    public:
        plugin_load_error(const std::string& context, const std::filesystem::path& path,
                          const std::string& details = "OS rejected linking.")
            : plugin_error("Plugin Load Error | " + context, path, details) {}
    };

    // Event structs
    struct load_request {
        std::filesystem::path virtual_path;
        std::string entry_point = "SandboxLibraryMain";
    };

} // namespace sandbox::events::plugins

// ============================================================================
// Plugin Macros (dissolved from macros/plugins.h)
// ============================================================================

#include "sandbox/event_bus/event_bus.h"

// Standard loader: Assumes "SandboxLibraryMain" as the C-linkage entry point
#define SANDBOX_PLUGIN_LOAD(ecs_ref, virtual_path) \
    sandbox::events::publish(ecs_ref, sandbox::events::plugins::load_request{virtual_path})

// Custom loader: Allows specifying a custom module entry point function name
#define SANDBOX_PLUGIN_LOAD_CUSTOM(ecs_ref, virtual_path, entry_point_name) \
    sandbox::events::publish(ecs_ref, sandbox::events::plugins::load_request{virtual_path, entry_point_name})
