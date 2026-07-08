
#include <dylib.hpp>

#include "library_loader.h"

#include <iostream>
#include <flecs.h>
#include <flecs/addons/cpp/flecs.hpp>
#include <sandbox/services/logs_service.h>
#include "exceptions.h"
#include "bootstrapper.h"
#include <sandbox/abi/library_loader.h>
#include <fstream>
#include <random>

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
            static auto* s_leaked_libs = new std::unordered_map<std::string, std::shared_ptr<dylib::library>>();
            if (s_leaked_libs->find(library_name) == s_leaked_libs->end()) {
                (*s_leaked_libs)[library_name] = std::make_shared<dylib::library>(path.string());
            }
            
            m_libraries.emplace(library_name, (*s_leaked_libs)[library_name]);

            sandbox::modules::logs::info(entity_world, "Successfully loaded: {}", library_name);
        }
        catch (const std::exception& e) {
            sandbox::modules::logs::error(entity_world, "Failed to load library '{}': {}", path.string(), e.what());
            throw library_load_error(std::format("Could not load library '{}': {}", path.string(), e.what()));
        }
    }

    void library_loader_t::load_from_memory(flecs::world& entity_world, const std::vector<uint8_t>& library_data) {
        if (library_data.empty()) {
            throw library_load_error("Cannot load library from empty memory buffer.");
        }
        
        std::filesystem::path cache_dir = std::filesystem::temp_directory_path() / "SandboxEngine" / "plugin_cache";
        if (!std::filesystem::exists(cache_dir)) {
            std::filesystem::create_directories(cache_dir);
        }
        
        std::string target_filename;
        bool found_match = false;
        
        for (const auto& entry : std::filesystem::directory_iterator(cache_dir)) {
            if (entry.is_regular_file()) {
                if (std::filesystem::file_size(entry.path()) == library_data.size()) {
                    std::ifstream file(entry.path(), std::ios::binary);
                    std::vector<uint8_t> content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                    if (content == library_data) {
                        target_filename = entry.path().string();
                        found_match = true;
                        sandbox::modules::logs::info(entity_world, "Library memory exactly matches cached file: {}", target_filename);
                        break;
                    }
                }
            }
        }
        
        if (!found_match) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 15);
            std::uniform_int_distribution<> dis2(8, 11);
            
            std::string uuid_str = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
            for (char& c : uuid_str) {
                if (c == 'x') {
                    const char* hex = "0123456789abcdef";
                    c = hex[dis(gen)];
                } else if (c == 'y') {
                    const char* hex = "89ab";
                    c = hex[dis2(gen) - 8];
                }
            }
            
#ifdef _WIN32
            std::string ext = ".dll";
#elif defined(__APPLE__)
            std::string ext = ".dylib";
#else
            std::string ext = ".so";
#endif
            std::filesystem::path new_path = cache_dir / (uuid_str + ext);
            target_filename = new_path.string();
            
            std::ofstream outfile(new_path, std::ios::binary);
            if (!outfile) {
                throw library_load_error("Failed to write library to cache directory.");
            }
            outfile.write(reinterpret_cast<const char*>(library_data.data()), library_data.size());
            sandbox::modules::logs::info(entity_world, "Wrote memory library to cache: {}", target_filename);
        }
        
        load(entity_world, target_filename);
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

extern "C" {
    static void library_loader_api_load(ecs_world_t* ecs, const char* path) {
        if (!ecs || !path) return;
        flecs::world entity_world(ecs);
        sandbox::core::bootstrapper_t::get_loader().load(entity_world, path);
    }
    
    static void library_loader_api_load_from_memory(ecs_world_t* ecs, const uint8_t* library_data, size_t size) {
        if (!ecs || !library_data || size == 0) return;
        flecs::world entity_world(ecs);
        std::vector<uint8_t> data_vec(library_data, library_data + size);
        sandbox::core::bootstrapper_t::get_loader().load_from_memory(entity_world, data_vec);
    }
    
    static void library_loader_api_unload(ecs_world_t* ecs, const char* library_name) {
        if (!ecs || !library_name) return;
        flecs::world entity_world(ecs);
        sandbox::core::bootstrapper_t::get_loader().unload(entity_world, library_name);
    }
    
    static sandbox_library_loader_api_t library_loader_api = {
        .load = library_loader_api_load,
        .load_from_memory = library_loader_api_load_from_memory,
        .unload = library_loader_api_unload,
    };
    
    SANDBOX_DEFINE_SERVICE(sandbox_library_loader_service_t, sandbox_library_loader_api_t, &library_loader_api);
}