#include <sandbox/sdk/engine.hpp>
#include <sandbox/abi/events.h>
#include "events.h"

extern "C" {
    static void events_publish(ecs_world_t* ecs, ecs_id_t event_id, const void* event) {
        flecs::world world(ecs);
        auto* evts = world.try_get_mut<sandbox::modules::events>();
        if (evts) {
            evts->publish(event_id, event);
        }
    }

    static ecs_entity_t events_subscribe(ecs_world_t* ecs, ecs_id_t event_id, sandbox_event_callback_t callback, void* user_data) {
        flecs::world world(ecs);
        auto* evts = world.try_get_mut<sandbox::modules::events>();
        if (evts) {
            try {
                return evts->subscribe(event_id, callback, user_data).id();
            } catch(...) {
                return 0;
            }
        }
        return 0;
    }

    static sandbox_events_api_t events_api = {
        .publish = events_publish,
        .subscribe = events_subscribe
    };

    SANDBOX_DEFINE_SERVICE(sandbox_events_service_t, sandbox_events_api_t, &events_api);
}

namespace sandbox::modules {
    SANDBOX_DECLARE_MODULE(events, {
        .struct_size = 0,
        .name = "events",
        .description = "Global event pub/sub module",
        .architecture = "sandbox::core",
        .version_major = 1,
        .version_minor = 0,
        .version_patch = 0,
        .service = &sandbox_events_service_t_info,
        .requirements = nullptr,
        .requirement_count = 0,
        .init_fn = nullptr
    });
}
