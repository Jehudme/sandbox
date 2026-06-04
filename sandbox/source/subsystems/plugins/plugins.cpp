#include "subsystems/plugins/plugins.h"
#include "sandbox/subsystems/filesystem/ifilesystem.h"
#include "sandbox/event_bus/plugin_events.h"
#include "sandbox/event_bus/event_bus.h"
#include "sandbox/event_bus/filesystem_events.h"
#include "sandbox/event_bus/logger_events.h"
#include "sandbox/utilities/filesystem.h"
#include "sandbox/core/plugin.h"

namespace sandbox::modules {

    plugins::plugins(world& ecs) : m_ecs(&ecs) {
        ecs.module<plugins>("::Modules::Plugins");
        ecs.set<sandbox::plugins_service>({this});

        // Bind the OS API pointers for Flecs before any dynamic libraries are loaded
        sandbox::configure_plugin_os_api();

        SANDBOX_INFO(ecs, "[Plugins] Dynamic linker subsystem operational.");
    }

    plugins::~plugins() = default;

    void plugins::load(std::string_view virtual_path, std::string_view entry_point) {

        // 1. Resolve virtual path to physical OS path
        std::filesystem::path physical_path = m_ecs->get<sandbox::filesystem_service>().api->absolute(virtual_path);

        if (physical_path.empty()) {
            SANDBOX_ERROR(*m_ecs, "[Plugins] VFS Resolution failed: {}", virtual_path);
            return; // CRITICAL FIX: Do not throw inside an observer!
        }

        // ====================================================================
        // CRITICAL FIX: Do NOT strip the extension!
        // We pass the exact physical path so dlopen gets the true filename.
        // ====================================================================
        std::string exact_path = physical_path.string();

        SANDBOX_DEBUG(*m_ecs, "[Plugins] Linking: {}::{}", physical_path.filename().string(), entry_point);

        // 3. Delegate to the OS dynamic linker via Flecs API
        auto library = ecs_import_from_library(m_ecs->c_ptr(), exact_path.c_str(), std::string(entry_point).c_str());

        // 4. Validate success
        if (library) {
            SANDBOX_INFO(*m_ecs, "[Plugins] Mounted: {}", physical_path.filename().string());
        } else {
            SANDBOX_ERROR(*m_ecs, "[Plugins] Mount failed: {}", physical_path.filename().string());
            // CRITICAL FIX: Do not throw! Just log the error and return safely.
            return;
        }
    }

} // namespace sandbox::modules
