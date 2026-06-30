#pragma once
#include <sandbox/abi/events.h>
#include <flecs/addons/cpp/flecs.hpp>

namespace sandbox::modules {
    class events {
    public:
        static void publish(flecs::world& world, ecs_id_t event_id, const void* event) {
            const sandbox_events_service_t* svc = SANDBOX_GET_SERVICE(world, sandbox_events_service_t);
            if (svc && svc->api) {
                svc->api->publish(world.c_ptr(), event_id, event);
            }
        }

        static flecs::entity subscribe(flecs::world& world, ecs_id_t event_id, sandbox_event_callback_t callback, void* user_data = nullptr) {
            const sandbox_events_service_t* svc = SANDBOX_GET_SERVICE(world, sandbox_events_service_t);
            if (svc && svc->api) {
                ecs_entity_t e = svc->api->subscribe(world.c_ptr(), event_id, callback, user_data);
                return flecs::entity(world, e);
            }
            return flecs::entity::null();
        }

        // C++ Type-Safe wrappers
        template<typename T>
        static void publish(flecs::world& world, const T* event) {
            publish(world, world.component<T>().id(), static_cast<const void*>(event));
        }

        template<typename T>
        static flecs::entity subscribe(flecs::world& world, void(*callback)(const T* event, void* user_data), void* user_data = nullptr) {
            return subscribe(world, world.component<T>().id(), reinterpret_cast<sandbox_event_callback_t>(callback), user_data);
        }
    };
}
