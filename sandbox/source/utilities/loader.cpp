#include "loader.h"
#include "sandbox/subsystems/filesystem/ifilesystem.h"
#include "sandbox/event_bus/event_bus.h"
#include "sandbox/event_bus/filesystem_events.h"
#include "sandbox/event_bus/logger_events.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/core/plugin.h"

namespace sandbox::internal {
    

    /// Resolves and dynamically links a plugin library into the active ECS world.
    std::expected<void, std::string> load(world& ecs, std::string_view virtual_path, std::string_view entry_point) {
        auto physical_path_res = ecs.get<sandbox::filesystem_service>().api->absolute(virtual_path);
        if (!physical_path_res) return std::unexpected(physical_path_res.error());
        std::filesystem::path physical_path = *physical_path_res;

        if (physical_path.empty()) {
            SANDBOX_ERROR(ecs, "[Loader] VFS Resolution failed: {}", virtual_path);
            return std::unexpected("VFS Resolution failed");
        }

        std::string exact_path = physical_path.string();

        SANDBOX_DEBUG(ecs, "[Loader] Linking: {}::{}", physical_path.filename().string(), entry_point);

        // Delegate to the OS dynamic linker via Flecs API
        auto library = ecs_import_from_library(ecs.c_ptr(), exact_path.c_str(), std::string(entry_point).c_str());

        // Validate success
        if (library) {
            SANDBOX_INFO(ecs, "[Loader] Mounted: {}", physical_path.filename().string());
            return {};
        } else {
            return std::unexpected(std::string("Plugin Error: Failed to find entry point ") + std::string(entry_point) + " in " + physical_path.string());
        }
    }

} // namespace sandbox::modules
