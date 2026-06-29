#pragma once
#include "properties.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque pointer representing sandbox::sdk::engine_t */
typedef struct sandbox_engine sandbox_engine_t;

/* Creates a new engine instance */
sandbox_engine_t* sandbox_engine_create(void);

/* Destroys the engine instance */
void sandbox_engine_destroy(sandbox_engine_t* engine);

/* Initializes the engine with the provided properties. Returns true on success. */
bool sandbox_engine_initialize(sandbox_engine_t* engine, sandbox_properties_handle_t properties);

/* Returns the ECS world managed by the engine, or NULL if not initialized/available */
void* sandbox_engine_get_ecs(sandbox_engine_t* engine);

#ifdef __cplusplus
}
#endif
