#pragma once
#include "../argument.hpp"

namespace sandbox {

    inline bool argument::has(ecs_world_t* ecs, const std::string& path) {
        return sandbox_argument_has(ecs, path.c_str());
    }

    inline bool argument::get_int64(ecs_world_t* ecs, const std::string& path, int64_t& out_val) {
        return sandbox_argument_get_int64(ecs, path.c_str(), &out_val);
    }

    inline bool argument::get_double(ecs_world_t* ecs, const std::string& path, double& out_val) {
        return sandbox_argument_get_double(ecs, path.c_str(), &out_val);
    }

    inline bool argument::get_bool(ecs_world_t* ecs, const std::string& path, bool& out_val) {
        return sandbox_argument_get_bool(ecs, path.c_str(), &out_val);
    }

    inline bool argument::get_string(ecs_world_t* ecs, const std::string& path, std::string& out_val) {
        bool found = false;
        struct Ctx { std::string* str; bool* found; } ctx = { &out_val, &found };
        
        sandbox_argument_read_string(ecs, path.c_str(), [](const char* value, void* user_data) {
            auto* c = static_cast<Ctx*>(user_data);
            if (value) {
                *c->str = value;
                *c->found = true;
            }
        }, &ctx);
        return found;
    }

    inline std::vector<std::string> argument::keys(ecs_world_t* ecs, const std::string& path) {
        std::vector<std::string> result;
        sandbox_argument_get_keys(ecs, path.c_str(), [](const char* key, void* ctx) {
            auto* vec = static_cast<std::vector<std::string>*>(ctx);
            vec->emplace_back(key);
        }, &result);
        return result;
    }

    inline properties argument::get_subtree(ecs_world_t* ecs, const std::string& path) {
        sandbox_properties_t* raw_sub = sandbox_argument_get_subtree(ecs, path.c_str());
        return properties(raw_sub);
    }

    template <typename Type>
    inline std::optional<Type> argument::get(ecs_world_t *ecs, const std::string &path) {
        // 1. Strings
        if constexpr (std::is_same_v<Type, std::string>) {
            std::string val;
            if (get_string(ecs, path, val)) return val;
        }
        // 2. Booleans (Must be checked before is_integral_v, as bool is technically an integral type)
        else if constexpr (std::is_same_v<Type, bool>) {
            bool val;
            if (get_bool(ecs, path, val)) return val;
        }
        // 3. Integers (Matches int64_t, int32_t, uint32_t, size_t, etc.)
        else if constexpr (std::is_integral_v<Type>) {
            int64_t val;
            if (get_int64(ecs, path, val)) return static_cast<Type>(val);
        }
        // 4. Floating Point (Matches double, float)
        else if constexpr (std::is_floating_point_v<Type>) {
            double val;
            if (get_double(ecs, path, val)) return static_cast<Type>(val);
        }
        // 5. Custom Subtrees
        else if constexpr (std::is_same_v<Type, properties>) {
            // Since get_subtree doesn't return a success boolean, we verify existence first
            if (has(ecs, path)) {
                return get_subtree(ecs, path);
            }
        }
        // 6. Unsupported Types
        else {
            // The !sizeof(Type*) delays the evaluation so this static_assert
            // only fires if the template is instantiated with a bad type.
            static_assert(!sizeof(Type*), "Unsupported type passed to sandbox::argument::get");
        }

        // Fallback for failure scenarios
        return std::nullopt;
    }

}
