#include "loader.h"
#include <sandbox/modules/filesystem/filesystem_api.h>
#include "sandbox/event_bus/event_bus.h"

#include <sandbox/modules/logger/logger_api.h>
#include "sandbox/utilities/filesystem.h"
#include "sandbox/core/plugin.h"

namespace sandbox::internal {
    

    /// Resolves and dynamically links a plugin library into the active ECS world.
    std::expected<void, std::string> load(world& ecs, std::string_view path) {
        std::string exact_path = std::string(path);

        // If it's not an absolute physical path, resolve it via VFS
        std::filesystem::path physical_path(exact_path);
        if (!physical_path.is_absolute()) {
            auto fs = sandbox::sdk::filesystem(ecs);
            auto absolute_res = fs.absolute(exact_path);
            
            if (!absolute_res) {
                return std::unexpected(absolute_res.error() + " for: " + exact_path);
            }
            exact_path = *absolute_res;
            physical_path = std::filesystem::path(exact_path);
        }

        SANDBOX_DEBUG(ecs, "Linking plugin: {}", physical_path.filename().string());

        // Delegate to the OS dynamic linker via the Flecs API
        auto library = ecs_import_from_library(ecs.c_ptr(), exact_path.c_str(), "SandboxLibraryMain");

        // Validate success
        if (library) {
            SANDBOX_INFO(ecs, "Linked plugin: {}", physical_path.filename().string());
            return {};
        } else {
            return std::unexpected(
                "Failed to find entry point 'SandboxLibraryMain' in '" + physical_path.string() + "'");
        }
    }

} // namespace sandbox::internal
