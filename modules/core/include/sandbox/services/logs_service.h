#pragma once
#include <utility>
#include <string>
#include <format>
#include <flecs.h>

#include <sandbox/abi/bootstrapper.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief API for the logs service.
 */
typedef struct {
    /**
     * @brief Logs a trace message.
     * @param ecs The entity component system world.
     * @param msg The message to log.
     */
    void (*trace)(ecs_world_t* ecs, const char* msg);
    
    /**
     * @brief Logs a debug message.
     * @param ecs The entity component system world.
     * @param msg The message to log.
     */
    void (*debug)(ecs_world_t* ecs, const char* msg);
    
    /**
     * @brief Logs an info message.
     * @param ecs The entity component system world.
     * @param msg The message to log.
     */
    void (*info)(ecs_world_t* ecs, const char* msg);
    
    /**
     * @brief Logs a warning message.
     * @param ecs The entity component system world.
     * @param msg The message to log.
     */
    void (*warn)(ecs_world_t* ecs, const char* msg);
    
    /**
     * @brief Logs an error message.
     * @param ecs The entity component system world.
     * @param msg The message to log.
     */
    void (*error)(ecs_world_t* ecs, const char* msg);
} sandbox_logs_api_t;

SANDBOX_API void sandbox_logs_trace(ecs_world_t* ecs, const char* msg);
SANDBOX_API void sandbox_logs_debug(ecs_world_t* ecs, const char* msg);
SANDBOX_API void sandbox_logs_info(ecs_world_t* ecs, const char* msg);
SANDBOX_API void sandbox_logs_warn(ecs_world_t* ecs, const char* msg);
SANDBOX_API void sandbox_logs_error(ecs_world_t* ecs, const char* msg);

/**
 * @brief The logging service definition.
 */
SANDBOX_DECLARE_SERVICE(sandbox_logs_service_t, sandbox_logs_api_t, {
    .name = "logs",
    .description = "The logging service",
    .architecture = "sandbox",
    .version_major = 1,
    .version_minor = 0
})

#ifdef __cplusplus
}
#endif
