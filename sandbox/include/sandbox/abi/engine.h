#pragma once
#include "properties.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque pointer representing the sandbox engine instance.
 */
typedef struct sandbox_engine sandbox_engine_t;

/**
 * @brief Creates a new engine instance.
 * @return A pointer to the new engine instance.
 */
sandbox_engine_t* sandbox_engine_create(void);

/**
 * @brief Destroys the engine instance.
 * @param engine The engine instance to destroy.
 */
void sandbox_engine_destroy(sandbox_engine_t* engine);

/**
 * @brief Initializes the engine with the provided properties.
 * @param engine The engine instance.
 * @param properties The initialization properties.
 * @return True on success, false otherwise.
 */
bool sandbox_engine_initialize(sandbox_engine_t* engine, sandbox_properties_handle_t properties);

/**
 * @brief Returns the ECS world managed by the engine.
 * @param engine The engine instance.
 * @return A pointer to the ECS world, or NULL if not initialized/available.
 */
void* sandbox_engine_get_ecs(sandbox_engine_t* engine);

#ifdef __cplusplus
}
#endif
