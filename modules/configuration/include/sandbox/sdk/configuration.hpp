#pragma once
#include <sandbox/abi/configuration.h>
#include <sandbox/abi/properties.h>
#include <flecs/addons/cpp/flecs.hpp>
#include <string>
#include <optional>
#include <vector>
#include <type_traits>

namespace sandbox::modules {
    class configuration {
    public:
        // C++ wrapper static method to get the raw properties handle
        static sandbox_properties_handle_t get_raw(flecs::world& world) {
            const sandbox_configuration_service_t* svc = SANDBOX_GET_SERVICE(world, sandbox_configuration_service_t);
            if (svc && svc->api) {
                return svc->api->get_properties();
            }
            sandbox_properties_handle_t invalid = {0};
            return invalid;
        }

        // C++ SDK template to safely get configuration values
        template <typename T>
        static std::optional<T> get(flecs::world& world, const std::string& path) {
            sandbox_properties_handle_t h = get_raw(world);
            if (!SANDBOX_HANDLE_IS_VALID(h)) return std::nullopt;

            if constexpr (std::is_same_v<T, std::string>) {
                std::optional<std::string> result;
                sandbox_properties_read_string(h, path.c_str(), [](const char* value, void* ctx) {
                    if (value) {
                        auto* res = static_cast<std::optional<std::string>*>(ctx);
                        *res = std::string(value);
                    }
                }, &result);
                return result;
            } else if constexpr (std::is_same_v<T, bool>) {
                bool val = false;
                if (sandbox_properties_get_bool(h, path.c_str(), &val)) return val;
            } else if constexpr (std::is_integral_v<T>) {
                int64_t val = 0;
                if (sandbox_properties_get_int64(h, path.c_str(), &val)) return static_cast<T>(val);
            } else if constexpr (std::is_floating_point_v<T>) {
                double val = 0.0;
                if (sandbox_properties_get_double(h, path.c_str(), &val)) return static_cast<T>(val);
            }
            return std::nullopt;
        }

        // C++ SDK template to safely set configuration values
        template <typename T>
        static void set(flecs::world& world, const std::string& path, const T& value) {
            sandbox_properties_handle_t h = get_raw(world);
            if (!SANDBOX_HANDLE_IS_VALID(h)) return;

            if constexpr (std::is_same_v<T, std::string>) {
                sandbox_properties_set_string(h, path.c_str(), value.c_str());
            } else if constexpr (std::is_same_v<T, const char*>) {
                sandbox_properties_set_string(h, path.c_str(), value);
            } else if constexpr (std::is_same_v<T, bool>) {
                sandbox_properties_set_bool(h, path.c_str(), value);
            } else if constexpr (std::is_integral_v<T>) {
                sandbox_properties_set_int64(h, path.c_str(), static_cast<int64_t>(value));
            } else if constexpr (std::is_floating_point_v<T>) {
                sandbox_properties_set_double(h, path.c_str(), static_cast<double>(value));
            }
        }
    };
}
