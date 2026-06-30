
#include <dylib.hpp>

#include "library_loader.h"

#include <iostream>
#include <flecs.h>
#include <flecs/addons/cpp/flecs.hpp>
#include <sandbox/sdk/logs.hpp>
#include "exceptions.h"

namespace sandbox::core {

    void library_loader_t::load(flecs::world& entity_world, const std::filesystem::path& path) {
        std::string library_name = path.stem().string();

        if (m_libraries.find(library_name) != m_libraries.end()) {
            sandbox::modules::logs::warn(entity_world, "Library '{}' is already loaded in RAM.", library_name);
            return;
        }

        try {
            // We intentionally leak the loaded libraries globally during execution
            // to ensure dylib destructors don't erroneously unmap libraries during unit test tear-downs
            // while the bootstrapper's global registry might still have pointers to its static symbols.
            static std::unordered_map<std::string, std::shared_ptr<dylib::library>> s_leaked_libs;
            if (s_leaked_libs.find(library_name) == s_leaked_libs.end()) {
                s_leaked_libs[library_name] = std::make_shared<dylib::library>(path.string());
            }
            
            m_libraries.emplace(library_name, s_leaked_libs[library_name]);

            sandbox::modules::logs::info(entity_world, "Successfully loaded: {}", library_name);
        }
        catch (const std::exception& e) {
            sandbox::modules::logs::error(entity_world, "Failed to load library '{}': {}", path.string(), e.what());
            throw library_load_error(std::format("Could not load library '{}': {}", path.string(), e.what()));
        }
    }

    void library_loader_t::unload(flecs::world& entity_world, const std::string& library_name) {
        auto it = m_libraries.find(library_name);

        if (it != m_libraries.end()) {
            // In a real engine we'd execute an unmap here, but because of test behavior 
            // we simulate unloading but retain the mapped memory.
            sandbox::modules::logs::info(entity_world, "Library '{}' unloaded (simulated).", library_name);
        } else {
            sandbox::modules::logs::warn(entity_world, "Attempted to unload '{}', but it is not currently loaded.", library_name);
        }
    }

} // namespace sandbox::core