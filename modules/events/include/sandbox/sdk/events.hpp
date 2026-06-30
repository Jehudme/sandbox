#pragma once
#include <sandbox/abi/events.h>
#include <flecs/addons/cpp/flecs.hpp>

namespace sandbox::modules {
    class events {
    public:
        explicit events(flecs::world& world) : m_world(world) {}

        void publish(ecs_id_t event_id, const void* event) {
            const sandbox_events_service_t* svc = SANDBOX_GET_SERVICE(m_world, sandbox_events_service_t);
            if (svc && svc->api) {
                svc->api->publish(m_world.c_ptr(), event_id, event);
            }
        }

        flecs::entity subscribe(ecs_id_t event_id, sandbox_event_callback_t callback, void* user_data = nullptr) {
            const sandbox_events_service_t* svc = SANDBOX_GET_SERVICE(m_world, sandbox_events_service_t);
            if (svc && svc->api) {
                ecs_entity_t e = svc->api->subscribe(m_world.c_ptr(), event_id, callback, user_data);
                return flecs::entity(m_world, e);
            }
            return flecs::entity::null();
        }
        
    private:
        flecs::world m_world;
    };
}
