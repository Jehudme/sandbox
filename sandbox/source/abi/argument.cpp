#include "sandbox/abi/argument.h"

ECS_COMPONENT_DECLARE(sandbox_argument_t);

extern "C" {

    bool sandbox_argument_has(ecs_world_t* ecs, const char* path) {
        if (!ecs) return false;
        const sandbox_argument_t* arg = (const sandbox_argument_t*)ecs_singleton_get(ecs, sandbox_argument_t);
        if (!arg || !arg->internal_properties) return false;
        return sandbox_properties_has(arg->internal_properties, path);
    }

    bool sandbox_argument_get_int64(ecs_world_t* ecs, const char* path, int64_t* out_val) {
        if (!ecs) return false;
        const sandbox_argument_t* arg = (const sandbox_argument_t*)ecs_singleton_get(ecs, sandbox_argument_t);
        if (!arg || !arg->internal_properties) return false;
        return sandbox_properties_get_int64(arg->internal_properties, path, out_val);
    }

    bool sandbox_argument_get_double(ecs_world_t* ecs, const char* path, double* out_val) {
        if (!ecs) return false;
        const sandbox_argument_t* arg = (const sandbox_argument_t*)ecs_singleton_get(ecs, sandbox_argument_t);
        if (!arg || !arg->internal_properties) return false;
        return sandbox_properties_get_double(arg->internal_properties, path, out_val);
    }

    bool sandbox_argument_get_bool(ecs_world_t* ecs, const char* path, bool* out_val) {
        if (!ecs) return false;
        const sandbox_argument_t* arg = (const sandbox_argument_t*)ecs_singleton_get(ecs, sandbox_argument_t);
        if (!arg || !arg->internal_properties) return false;
        return sandbox_properties_get_bool(arg->internal_properties, path, out_val);
    }

    void sandbox_argument_read_string(ecs_world_t* ecs, const char* path, void (*callback)(const char* value, void* user_data), void* user_data) {
        if (!callback) return;
        if (!ecs) {
            callback(nullptr, user_data);
            return;
        }
        const sandbox_argument_t* arg = (const sandbox_argument_t*)ecs_singleton_get(ecs, sandbox_argument_t);
        if (!arg || !arg->internal_properties) {
            callback(nullptr, user_data);
            return;
        }
        sandbox_properties_read_string(arg->internal_properties, path, callback, user_data);
    }

    void sandbox_argument_get_keys(ecs_world_t* ecs, const char* path, void (*callback)(const char* key, void* ctx), void* ctx) {
        if (!ecs) return;
        const sandbox_argument_t* arg = (const sandbox_argument_t*)ecs_singleton_get(ecs, sandbox_argument_t);
        if (!arg || !arg->internal_properties) return;
        sandbox_properties_keys(arg->internal_properties, path, callback, ctx);
    }

    sandbox_properties_t* sandbox_argument_get_subtree(ecs_world_t* ecs, const char* path) {
        if (!ecs) return nullptr;
        const sandbox_argument_t* arg = (const sandbox_argument_t*)ecs_singleton_get(ecs, sandbox_argument_t);
        if (!arg || !arg->internal_properties) return nullptr;
        return sandbox_properties_sub(arg->internal_properties, path);
    }

}
