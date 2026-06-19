
#include <dylib.hpp>

#include "library_loader.h"

#include <iostream>

namespace sandbox::core {

    void library_loader_t::load(const std::filesystem::path& path) {
        // Extract the filename without extension to use as the map key (e.g., "GameLogic")
        std::string library_name = path.stem().string();

        // Prevent loading the exact same library twice and leaking memory
        if (m_libraries.find(library_name) != m_libraries.end()) {
            std::cerr << "[Loader] Library '" << library_name << "' is already loaded in RAM.\n";
            return;
        }

        try {
            // Emplace constructs the dylib::library in-place inside the map.
            // In dylib v3.0+, passing the path to the constructor automatically opens it.
            m_libraries.emplace(library_name, dylib::library(path.string()));

            std::cerr << "[Loader] Successfully loaded: " << library_name << "\n";
        }
        catch (const std::exception& e) {
            // Catches dylib::load_error, std::invalid_argument (invalid/missing path),
            // and any other OS-level dynamic-linker errors from dylib v3.
            std::cerr << "[Loader] Failed to load library at '" << path.string()
                      << "'.\nReason: " << e.what() << "\n";
        }
    }

    void library_loader_t::unload(const std::string& library_name) {
        auto it = m_libraries.find(library_name);

        if (it != m_libraries.end()) {
            // Erasing the element from the map instantly triggers the dylib::library destructor.
            // The destructor safely executes dlclose() or FreeLibrary() for you.
            m_libraries.erase(it);

            std::cerr << "[Loader] Successfully unloaded: " << library_name << "\n";
        } else {
            std::cerr << "[Loader] Warning: Attempted to unload '" << library_name
                      << "', but it is not currently loaded.\n";
        }
    }

} // namespace sandbox::core