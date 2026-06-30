#pragma once

#include <flecs.h>
#include <sandbox/abi/events.h>

namespace sandbox::modules {

    struct EventCallbackData {
        sandbox_event_callback_t cb;
        void* user_data;
    };

    struct SubscribesTo {};

    class events {
    public:
        explicit events(flecs::world& world);
        ~events();

        events(const events&) = delete;
        events& operator=(const events&) = delete;

        void publish(ecs_id_t event_id, const void* event);
        flecs::entity subscribe(ecs_id_t event_id, sandbox_event_callback_t callback, void* user_data = nullptr);

    private:
        flecs::world m_world;
    };

} // namespace sandbox::modules
