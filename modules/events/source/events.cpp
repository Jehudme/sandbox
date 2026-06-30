#include "events.h"
#include <sandbox/sdk/logs.hpp>
#include "../../../sandbox/source/core/exceptions.h"
#include <stdexcept>

namespace sandbox::modules {

    events::events(flecs::world& world) : m_world(world) {
        sandbox::modules::logs::trace(m_world, "Events Module Initializing...");
        
        m_world.component<EventCallbackData>();
        m_world.component<SubscribesTo>();
    }

    events::~events() = default;

    void events::publish(ecs_id_t event_id, const void* event) {
        try {
            auto q = m_world.query_builder<EventCallbackData>()
                .with<SubscribesTo>(event_id)
                .build();
            
            q.each([&](EventCallbackData& data) {
                if (data.cb) {
                    data.cb(event, data.user_data);
                }
            });
        } catch (const std::exception& e) {
            sandbox::modules::logs::error(m_world, "Exception during events::publish: {}", e.what());
        } catch (...) {
            sandbox::modules::logs::error(m_world, "Unknown exception during events::publish");
        }
    }

    flecs::entity events::subscribe(ecs_id_t event_id, sandbox_event_callback_t callback, void* user_data) {
        if (!callback) {
            sandbox::modules::logs::error(m_world, "Null callback provided to events::subscribe");
            throw std::invalid_argument("Null callback");
        }
        
        flecs::entity e = m_world.entity();
        e.add<SubscribesTo>(event_id);
        e.set<EventCallbackData>({callback, user_data});
        
        return e;
    }

} // namespace sandbox::modules
