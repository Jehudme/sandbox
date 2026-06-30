#pragma once

#include <sandbox/abi/logs.h>
#include <flecs.h>
#include <format>
#include <string>
#include <utility>

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