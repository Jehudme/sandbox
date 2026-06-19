#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <flecs.h>

#include "sandbox/core/platform.h"
#include "sandbox/core/properties.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Component structure to hold arguments pointer (internal usage) */
typedef struct {
    sandbox_properties_t* internal_properties;
} sandbox_argument_t;

extern ECS_COMPONENT_DECLARE(sandbox_argument_t);

/* ========================================================================== */
/* GETTERS                                                                    */
/* ========================================================================== */
SANDBOX_API bool sandbox_argument_has(ecs_world_t* ecs, const char* path);

/* These functions return true on success, and populate out_val. If they fail, out_val is unmodified */
SANDBOX_API bool sandbox_argument_get_int64(ecs_world_t* ecs, const char* path, int64_t* out_val);
SANDBOX_API bool sandbox_argument_get_double(ecs_world_t* ecs, const char* path, double* out_val);
SANDBOX_API bool sandbox_argument_get_bool(ecs_world_t* ecs, const char* path, bool* out_val);

/* Invokes the callback with the string pointer. If not found or type mismatch, invokes with nullptr. Pointer is only valid during the callback. */
SANDBOX_API void sandbox_argument_read_string(ecs_world_t* ecs, const char* path, void (*callback)(const char* value, void* user_data), void* user_data);

/* ========================================================================== */
/* SUBTREE & KEYS                                                             */
/* ========================================================================== */

/* Iterates over all keys at the given path. Calls the callback for each key. */
SANDBOX_API void sandbox_argument_get_keys(ecs_world_t* ecs, const char* path, void (*callback)(const char* key, void* ctx), void* ctx);

/* Returns a dynamically allocated property object. Must be freed with sandbox_properties_destroy */
SANDBOX_API sandbox_properties_t* sandbox_argument_get_subtree(ecs_world_t* ecs, const char* path);

#ifdef __cplusplus
}
#endif
