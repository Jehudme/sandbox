#pragma once

#include <sandbox/abi/logs.h>
#include <flecs/addons/cpp/flecs.hpp>

namespace sandbox::modules {
    class logs {
    public:
        static void trace(flecs::world& world, const char* msg) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_logs_service_t)) {
                if (svc->api) svc->api->trace(world.c_ptr(), msg);
            }
        }
        
        static void debug(flecs::world& world, const char* msg) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_logs_service_t)) {
                if (svc->api) svc->api->debug(world.c_ptr(), msg);
            }
        }
        
        static void info(flecs::world& world, const char* msg) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_logs_service_t)) {
                if (svc->api) svc->api->info(world.c_ptr(), msg);
            }
        }
        
        static void warn(flecs::world& world, const char* msg) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_logs_service_t)) {
                if (svc->api) svc->api->warn(world.c_ptr(), msg);
            }
        }
        
        static void error(flecs::world& world, const char* msg) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_logs_service_t)) {
                if (svc->api) svc->api->error(world.c_ptr(), msg);
            }
        }
    };
}
