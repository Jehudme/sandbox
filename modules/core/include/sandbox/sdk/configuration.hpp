#pragma once
#include <sandbox/abi/configuration.h>
#include <sandbox/sdk/properties.hpp>
#include <flecs/addons/cpp/flecs.hpp>
#include <string>
#include <optional>
#include <vector>
#include <type_traits>

namespace sandbox::modules {
    class configuration {
    public:
        // C++ wrapper static method to get a temporary properties object
        // Note: The caller should NOT hold onto this beyond the immediate scope, 
        // as we release the handle to prevent destruction of the underlying C handle.
        static sandbox::properties get_properties(flecs::world& world) {
            const sandbox_configuration_service_t* svc = SANDBOX_GET_SERVICE(world, sandbox_configuration_service_t);
            if (svc && svc->api) {
                return sandbox::properties(svc->api->get_properties(world.c_ptr()));
            }
            sandbox_properties_handle_t invalid = {0};
            return sandbox::properties(invalid);
        }

        // C++ SDK template to safely get configuration values
        template <typename T>
        static std::optional<T> get(flecs::world& world, const std::string& path) {
            sandbox::properties temp = get_properties(world);
            if (!temp.is_valid()) return std::nullopt;
            auto res = temp.get<T>(path);
            temp.release(); // Do not destroy the module's handle
            return res;
        }

        // C++ SDK template to safely set configuration values
        template <typename T>
        static void set(flecs::world& world, const std::string& path, const T& value) {
            sandbox::properties temp = get_properties(world);
            if (!temp.is_valid()) return;
            temp.set<T>(path, value);
            temp.release();
        }
    };
}
