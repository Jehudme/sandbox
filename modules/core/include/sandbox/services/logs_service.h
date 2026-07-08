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

#ifdef __cplusplus
namespace sandbox::modules {
    /**
     * @brief High-level C++ SDK for interacting with the logging module.
     */
    class logs {
    public:
        /**
         * @brief Logs a trace message.
         * @tparam Args Format argument types.
         * @param entity_world The flecs world.
         * @param fmt The format string.
         * @param args The format arguments.
         */
        template <typename... Args>
        static void trace(flecs::world& entity_world, std::format_string<Args...> fmt, Args&&... args) {
            if (const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_logs_service_t)) {
                if (service->api) {
                    std::string message = std::format(fmt, std::forward<Args>(args)...);
                    service->api->trace(entity_world.c_ptr(), message.c_str());
                }
            }
        }

        /**
         * @brief Logs a debug message.
         * @tparam Args Format argument types.
         * @param entity_world The flecs world.
         * @param fmt The format string.
         * @param args The format arguments.
         */
        template <typename... Args>
        static void debug(flecs::world& entity_world, std::format_string<Args...> fmt, Args&&... args) {
            if (const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_logs_service_t)) {
                if (service->api) {
                    std::string message = std::format(fmt, std::forward<Args>(args)...);
                    service->api->debug(entity_world.c_ptr(), message.c_str());
                }
            }
        }

        /**
         * @brief Logs an informational message.
         * @tparam Args Format argument types.
         * @param entity_world The flecs world.
         * @param fmt The format string.
         * @param args The format arguments.
         */
        template <typename... Args>
        static void info(flecs::world& entity_world, std::format_string<Args...> fmt, Args&&... args) {
            if (const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_logs_service_t)) {
                if (service->api) {
                    std::string message = std::format(fmt, std::forward<Args>(args)...);
                    service->api->info(entity_world.c_ptr(), message.c_str());
                }
            }
        }

        /**
         * @brief Logs a warning message.
         * @tparam Args Format argument types.
         * @param entity_world The flecs world.
         * @param fmt The format string.
         * @param args The format arguments.
         */
        template <typename... Args>
        static void warn(flecs::world& entity_world, std::format_string<Args...> fmt, Args&&... args) {
            if (const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_logs_service_t)) {
                if (service->api) {
                    std::string message = std::format(fmt, std::forward<Args>(args)...);
                    service->api->warn(entity_world.c_ptr(), message.c_str());
                }
            }
        }

        /**
         * @brief Logs an error message.
         * @tparam Args Format argument types.
         * @param entity_world The flecs world.
         * @param fmt The format string.
         * @param args The format arguments.
         */
        template <typename... Args>
        static void error(flecs::world& entity_world, std::format_string<Args...> fmt, Args&&... args) {
            if (const auto* service = SANDBOX_GET_SERVICE(entity_world, sandbox_logs_service_t)) {
                if (service->api) {
                    std::string message = std::format(fmt, std::forward<Args>(args)...);
                    service->api->error(entity_world.c_ptr(), message.c_str());
                }
            }
        }
    };
}
#endif
