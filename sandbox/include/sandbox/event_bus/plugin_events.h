#pragma once

#include <filesystem>
#include <string>
#include <stdexcept>
#include "sandbox/core/platform.h"

namespace sandbox::events::plugins {

    // MARK: - Plugin Exception Classes
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

} // namespace sandbox::events::plugins

// MARK: - Plugin Macros

#include "sandbox/subsystems/plugins/iplugins.h"

// Standard loader: Assumes "SandboxLibraryMain" as the C-linkage entry point
#define SANDBOX_PLUGIN_LOAD(ecs_ref, virtual_path) \
    do { \
        if ((ecs_ref).has<sandbox::plugins_service>()) { \
            (ecs_ref).get<sandbox::plugins_service>().api->load(virtual_path); \
        } \
    } while(0)

// Custom loader: Allows specifying a custom module entry point function name
#define SANDBOX_PLUGIN_LOAD_CUSTOM(ecs_ref, virtual_path, entry_point_name) \
    do { \
        if ((ecs_ref).has<sandbox::plugins_service>()) { \
            (ecs_ref).get<sandbox::plugins_service>().api->load(virtual_path, entry_point_name); \
        } \
    } while(0)
