#include "loader.h"
#include "sandbox/subsystems/filesystem/ifilesystem.h"
#include "sandbox/event_bus/event_bus.h"

#include "sandbox/subsystems/logger/ilogger.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/core/plugin.h"

namespace sandbox::internal {
    

    /// Resolves and dynamically links a plugin library into the active ECS world.
    std::expected<void, std::string> load(world& ecs, std::string_view virtual_path) {
        sandbox_payload path_payload{};
        int32_t res = ecs.get<sandbox::filesystem_service>().api->absolute(std::string(virtual_path).c_str(), &path_payload);
        if (res != 0) return std::unexpected("Absolute path resolution failed for: " + std::string(virtual_path));

        if (!path_payload.bytes) {
            return std::unexpected(std::string("VFS resolution returned empty for: ") + std::string(virtual_path));
        }

        std::string exact_path(reinterpret_cast<const char*>(path_payload.bytes));
        if (path_payload.free_func) path_payload.free_func(path_payload.bytes);

        std::filesystem::path physical_path(exact_path);
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
