#pragma once
#include <sandbox/abi/configuration.h>
#include <sandbox/sdk/properties.hpp>
#include <flecs/addons/cpp/flecs.hpp>
#include <string>
#include <optional>
#include <vector>
#include <type_traits>

namespace sandbox::modules {
    /**
     * @brief High-level C++ SDK for interacting with the configuration module.
     */
    class configuration {
    public:
        /**
         * @brief Retrieves a temporary properties object containing the configuration.
         * @param entity_world The flecs world.
         * @return The properties object representing the configuration.
         */
        static sandbox::properties get_properties(flecs::world& entity_world) {
            const sandbox_configuration_service_t* service = SANDBOX_GET_SERVICE(entity_world, sandbox_configuration_service_t);
            if (service && service->api && service->api->get_properties) {
                return sandbox::properties(service->api->get_properties(entity_world.c_ptr()), false);
            }
            sandbox_properties_handle_t invalid = {0};
            return sandbox::properties(invalid, false);
        }

        /**
         * @brief Retrieves a configuration value at the given path.
         * @tparam Type The expected value type.
         * @param entity_world The flecs world.
         * @param path The key path (e.g. "logs/level").
         * @return An optional containing the value if found, or nullopt.
         */
        template <typename Type>
        static std::optional<Type> get(flecs::world& entity_world, const std::string& path) {
            sandbox::properties temp = get_properties(entity_world);
            if (!temp.is_valid()) return std::nullopt;
            auto result = temp.get<Type>(path);
            return result;
        }

        /**
         * @brief Sets a configuration value at the given path.
         * @tparam Type The value type.
         * @param entity_world The flecs world.
         * @param path The key path.
         * @param value The value to set.
         */
        template <typename Type>
        static void set(flecs::world& entity_world, const std::string& path, const Type& value) {
            sandbox::properties temp = get_properties(entity_world);
            if (!temp.is_valid()) return;
            temp.set<Type>(path, value);
        }
    };
}
