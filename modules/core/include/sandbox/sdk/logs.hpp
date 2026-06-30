#pragma once

#include <sandbox/abi/logs.h>
#include <flecs.h>
#include <format>
#include <string>
#include <utility>

namespace sandbox::modules {
    class logs {
    public:
        template <typename... Args>
        static void trace(flecs::world& world, std::format_string<Args...> fmt, Args&&... args) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_logs_service_t)) {
                if (svc->api) {
                    std::string msg = std::format(fmt, std::forward<Args>(args)...);
                    svc->api->trace(world.c_ptr(), msg.c_str());
                }
            }
        }

        template <typename... Args>
        static void debug(flecs::world& world, std::format_string<Args...> fmt, Args&&... args) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_logs_service_t)) {
                if (svc->api) {
                    std::string msg = std::format(fmt, std::forward<Args>(args)...);
                    svc->api->debug(world.c_ptr(), msg.c_str());
                }
            }
        }

        template <typename... Args>
        static void info(flecs::world& world, std::format_string<Args...> fmt, Args&&... args) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_logs_service_t)) {
                if (svc->api) {
                    std::string msg = std::format(fmt, std::forward<Args>(args)...);
                    svc->api->info(world.c_ptr(), msg.c_str());
                }
            }
        }

        template <typename... Args>
        static void warn(flecs::world& world, std::format_string<Args...> fmt, Args&&... args) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_logs_service_t)) {
                if (svc->api) {
                    std::string msg = std::format(fmt, std::forward<Args>(args)...);
                    svc->api->warn(world.c_ptr(), msg.c_str());
                }
            }
        }

        template <typename... Args>
        static void error(flecs::world& world, std::format_string<Args...> fmt, Args&&... args) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_logs_service_t)) {
                if (svc->api) {
                    std::string msg = std::format(fmt, std::forward<Args>(args)...);
                    svc->api->error(world.c_ptr(), msg.c_str());
                }
            }
        }
    };
}