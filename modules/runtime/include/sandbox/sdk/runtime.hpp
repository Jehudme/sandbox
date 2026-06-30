#pragma once

#include <sandbox/abi/runtime.h>
#include <flecs.h>

namespace sandbox::modules {
    class runtime {
    public:
        static void run(flecs::world& world) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_runtime_service_t)) {
                if (svc->api && svc->api->run) {
                    svc->api->run(world.c_ptr());
                }
            }
        }

        static void start(flecs::world& world) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_runtime_service_t)) {
                if (svc->api && svc->api->start) {
                    svc->api->start(world.c_ptr());
                }
            }
        }

        static void stop(flecs::world& world) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_runtime_service_t)) {
                if (svc->api && svc->api->stop) {
                    svc->api->stop(world.c_ptr());
                }
            }
        }

        static void pause(flecs::world& world) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_runtime_service_t)) {
                if (svc->api && svc->api->pause) {
                    svc->api->pause(world.c_ptr());
                }
            }
        }

        static void resume(flecs::world& world) {
            if (const auto* svc = SANDBOX_GET_SERVICE(world, sandbox_runtime_service_t)) {
                if (svc->api && svc->api->resume) {
                    svc->api->resume(world.c_ptr());
                }
            }
        }
    };
}
