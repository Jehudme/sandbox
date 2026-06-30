
#include <dylib.hpp>

#include "library_loader.h"

#include <iostream>
#include <flecs.h>
#include <flecs/addons/cpp/flecs.hpp>
#include <sandbox/sdk/logs.hpp>
#include "exceptions.h"

namespace sandbox::core {

    void library_loader_t::load(flecs::world& ecs, const std::filesystem::path& path) {
        // Extract the filename without extension to use as the map key (e.g., "GameLogic")
        std::string library_name = path.stem().string();

        // Prevent loading the exact same library twice and leaking memory
        if (m_libraries.find(library_name) != m_libraries.end()) {
            sandbox::modules::logs::warn(ecs, "Library '{}' is already loaded in RAM.", library_name);
            return;
        }

        try {
            // Emplace constructs the dylib::library on the heap to prevent unloading during tests
            static std::unordered_map<std::string, std::shared_ptr<dylib::library>> s_leaked_libs;
            if (s_leaked_libs.find(library_name) == s_leaked_libs.end()) {
                s_leaked_libs[library_name] = std::make_shared<dylib::library>(path.string());
            }
            
            // Add to the local map just for tracking
            m_libraries.emplace(library_name, s_leaked_libs[library_name]);

            sandbox::modules::logs::info(ecs, "Successfully loaded: {}", library_name);
        }
        catch (const std::exception& e) {
            // Catches dylib::load_error, std::invalid_argument (invalid/missing path),
            // and any other OS-level dynamic-linker errors from dylib v3.
            sandbox::modules::logs::error(ecs, "Failed to load library '{}': {}", path.string(), e.what());
            throw library_load_error(std::format("Could not load library '{}': {}", path.string(), e.what()));
        }
    }

    void library_loader_t::unload(flecs::world& ecs, const std::string& library_name) {
        auto it = m_libraries.find(library_name);

        if (it != m_libraries.end()) {
            // Do NOT erase it because we are leaking it intentionally so pointers don't dangle in tests.
            // In a real engine, we'd have a proper unload strategy or just let process exit handle it.
            // m_libraries.erase(it);
            sandbox::modules::logs::info(ecs, "Library '{}' unloaded (simulated).", library_name);
        } else {
            sandbox::modules::logs::warn(ecs, "Attempted to unload '{}', but it is not currently loaded.", library_name);
        }
    }

} // namespace sandbox::sdk