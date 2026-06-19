#pragma once
#include "../argument.hpp"

namespace sandbox {

    inline bool Argument::has(ecs_world_t* ecs, const std::string& path) {
        return sandbox_argument_has(ecs, path.c_str());
    }

    inline bool Argument::get_int64(ecs_world_t* ecs, const std::string& path, int64_t& out_val) {
        return sandbox_argument_get_int64(ecs, path.c_str(), &out_val);
    }

    inline bool Argument::get_double(ecs_world_t* ecs, const std::string& path, double& out_val) {
        return sandbox_argument_get_double(ecs, path.c_str(), &out_val);
    }

    inline bool Argument::get_bool(ecs_world_t* ecs, const std::string& path, bool& out_val) {
        return sandbox_argument_get_bool(ecs, path.c_str(), &out_val);
    }

    inline bool Argument::get_string(ecs_world_t* ecs, const std::string& path, std::string& out_val) {
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

    inline std::vector<std::string> Argument::keys(ecs_world_t* ecs, const std::string& path) {
        std::vector<std::string> result;
        sandbox_argument_get_keys(ecs, path.c_str(), [](const char* key, void* ctx) {
            auto* vec = static_cast<std::vector<std::string>*>(ctx);
            vec->emplace_back(key);
        }, &result);
        return result;
    }

    inline Properties Argument::get_subtree(ecs_world_t* ecs, const std::string& path) {
        sandbox_properties_t* raw_sub = sandbox_argument_get_subtree(ecs, path.c_str());
        return Properties(raw_sub);
    }

}
