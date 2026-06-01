#include "modules/plugins.h"
#include "sandbox/exceptions/plugins.h"
#include "sandbox/utilities/events.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/macros/logger.h"
#include "sandbox/macros/vfs.h"
#include "sandbox/core/plugin.h"

namespace sandbox::modules {

    plugins::plugins(world& ecs) {
        ecs.module<plugins>("::Modules::Plugins");

        // Bind the OS API pointers for Flecs before any dynamic libraries are loaded
        sandbox::configure_plugin_os_api();

        sandbox::events::subscribe<events::plugins::load_request>(ecs, [this, &ecs](const auto& e) { on_load(ecs, e); });

        SANDBOX_INFO(ecs, "[Plugins] Dynamic linker subsystem operational.");
    }

    plugins::~plugins() = default;

    void plugins::on_load(world& ecs, const events::plugins::load_request& e) {
        // 1. Resolve virtual path to physical OS path
        std::filesystem::path physical_path = SANDBOX_VFS_EXEC_ABSOLUTE(ecs, e.virtual_path);

        if (physical_path.empty()) {
            SANDBOX_ERROR(ecs, "[Plugins] VFS Resolution failed: {}", e.virtual_path.string());
            return; // CRITICAL FIX: Do not throw inside an observer!
        }

        // ====================================================================
        // CRITICAL FIX: Do NOT strip the extension!
        // We pass the exact physical path so dlopen gets the true filename.
        // ====================================================================
        std::string exact_path = physical_path.string();

        SANDBOX_DEBUG(ecs, "[Plugins] Linking: {}::{}", physical_path.filename().string(), e.entry_point);

        // 3. Delegate to the OS dynamic linker via Flecs API
        auto library = ecs_import_from_library(ecs.c_ptr(), exact_path.c_str(), e.entry_point.c_str());

        // 4. Validate success
        if (library) {
            SANDBOX_INFO(ecs, "[Plugins] Mounted: {}", physical_path.filename().string());
        } else {
            SANDBOX_ERROR(ecs, "[Plugins] Mount failed: {}", physical_path.filename().string());
            // CRITICAL FIX: Do not throw! Just log the error and return safely.
            return;
        }
    }

} // namespace sandbox::modules