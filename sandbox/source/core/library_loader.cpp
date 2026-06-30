
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
            // Emplace constructs the dylib::library in-place inside the map.
            // In dylib v3.0+, passing the path to the constructor automatically opens it.
            m_libraries.emplace(library_name, dylib::library(path.string()));

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
            // Erasing the element from the map instantly triggers the dylib::library destructor.
            // The destructor safely executes dlclose() or FreeLibrary() for you.
            m_libraries.erase(it);

            sandbox::modules::logs::info(ecs, "Successfully unloaded: {}", library_name);
        } else {
            sandbox::modules::logs::warn(ecs, "Attempted to unload '{}', but it is not currently loaded.", library_name);
        }
    }

} // namespace sandbox::sdk