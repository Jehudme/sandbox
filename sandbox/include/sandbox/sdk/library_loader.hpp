#pragma once

#include <filesystem>
#include <sandbox/abi/library_loader.h>
#include <flecs/addons/cpp/flecs.hpp>
#include <vector>

namespace sandbox::core {

    /**
     * @brief SDK wrapper for the library loader service.
     */
    class library_loader {
    public:
        /**
         * @brief Loads a library from a file path.
         * @param entity_world The flecs world.
         * @param path The path to the library.
         */
        static void load(flecs::world& entity_world, std::filesystem::path path) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_library_loader_service_t);
            if (service && service->api && service->api->load) {
                service->api->load(entity_world.c_ptr(), path.string().c_str());
            }
        }

        /**
         * @brief Loads a library from a memory buffer.
         * @param entity_world The flecs world.
         * @param library_data The memory buffer containing the library.
         */
        static void load_from_memory(flecs::world& entity_world, const std::vector<uint8_t>& library_data) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_library_loader_service_t);
            if (service && service->api && service->api->load_from_memory) {
                service->api->load_from_memory(entity_world.c_ptr(), library_data.data(), library_data.size());
            }
        }

        /**
         * @brief Unloads a library by name.
         * @param entity_world The flecs world.
         * @param library_name The name of the library to unload.
         */
        static void unload(flecs::world& entity_world, const char* library_name) {
            const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_library_loader_service_t);
            if (service && service->api && service->api->unload) {
                service->api->unload(entity_world.c_ptr(), library_name);
            }
        }
    };

} // namespace sandbox::core
