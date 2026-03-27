#include <sandbox/sandbox.h>
#include <thread>
#include <chrono>

#include "sandbox/reflections/registration.h"

struct position
{
    float x;
    float y;
};

struct velocity
{
    float dx;
    float dy;
};

struct player_spawned_event
{
    std::string player_name;
    float spawn_x;
    float spawn_y;
};

int main()
{
    SANDBOX_LOG_INFO("=== Starting Extension Tests ===");

    sandbox::properties config;
    config.set({"logger", "level"}, std::string("TRACE"));

    sandbox::engine app;
    app.initialize(config);

    SANDBOX_LOG_INFO("=== Testing extensions::logger ===");

    auto* ext_logger = app.get_extension<sandbox::extensions::logger>("logger");
    if (ext_logger)
    {
        ext_logger->info("extensions::logger is working correctly");
        ext_logger->warn("this is a warning from extensions::logger");
        ext_logger->debug("debug message from extensions::logger");
    }

    SANDBOX_LOG_INFO("=== Testing extensions::scopes ===");

    auto* ext_scopes = app.get_extension<sandbox::extensions::scopes>("scopes");
    if (ext_scopes)
    {
        ext_scopes->set_scope({"game", "world"});
        ext_scopes->push_scope({"game", "entities"});
        ext_scopes->set_scope({"game"});
    }

    SANDBOX_LOG_INFO("=== Testing extensions::stages ===");

    auto* ext_stages = app.get_extension<sandbox::extensions::stages>("stages");
    if (ext_stages)
    {
        ext_stages->create("physics_stage");
        ext_stages->create("render_stage", {"physics_stage"});
        ext_stages->create("ai_stage", {"physics_stage"});

        SANDBOX_LOG_INFO("physics_stage exists: {}", ext_stages->exists("physics_stage"));
        SANDBOX_LOG_INFO("render_stage exists: {}", ext_stages->exists("render_stage"));

        ext_stages->disable("ai_stage");
        SANDBOX_LOG_INFO("ai_stage enabled after disable: {}", ext_stages->enabled("ai_stage"));

        ext_stages->enable("ai_stage");
        SANDBOX_LOG_INFO("ai_stage enabled after enable: {}", ext_stages->enabled("ai_stage"));
    }

    SANDBOX_LOG_INFO("=== Testing extensions::storage ===");

    auto* ext_storage = app.get_extension<sandbox::extensions::storage>("storage");
    if (ext_storage)
    {
        ext_storage->create<position>("player_1", 0.0f, 0.0f);
        ext_storage->create<position>("player_2", 100.0f, 50.0f);
        ext_storage->create<velocity>("player_1_vel", 1.0f, 0.5f);

        SANDBOX_LOG_INFO("player_1 exists: {}", ext_storage->exists("player_1"));

        auto* player_1_position = ext_storage->get<position>("player_1");
        if (player_1_position)
        {
            SANDBOX_LOG_INFO("player_1 position: x={:.2f} y={:.2f}", player_1_position->x, player_1_position->y);
        }

        auto* player_2_position = ext_storage->get<position>("player_2");
        if (player_2_position)
        {
            SANDBOX_LOG_INFO("player_2 position: x={:.2f} y={:.2f}", player_2_position->x, player_2_position->y);
        }

        ext_storage->destroy("player_1_vel");
        SANDBOX_LOG_INFO("player_1_vel exists after destroy: {}", ext_storage->exists("player_1_vel"));
    }

    SANDBOX_LOG_INFO("=== Testing extensions::systems ===");

    auto* ext_systems = app.get_extension<sandbox::extensions::systems>("systems");
    if (ext_systems)
    {
        ext_systems->create<position>("movement_system", "physics_stage",
            [](auto& builder) {
                builder.interval(0.1f);
            },
            [](position& pos) {
                pos.x += 0.1f;
                pos.y += 0.05f;
            }
        );

        ext_systems->create<position, velocity>("physics_system", "physics_stage",
            [](position& pos, const velocity& vel) {
                pos.x += vel.dx;
                pos.y += vel.dy;
            }
        );

        SANDBOX_LOG_INFO("movement_system exists: {}", ext_systems->exists("movement_system"));
        SANDBOX_LOG_INFO("physics_system exists: {}", ext_systems->exists("physics_system"));

        ext_systems->disable("physics_system");
        SANDBOX_LOG_INFO("physics_system enabled after disable: {}", ext_systems->enabled("physics_system"));

        ext_systems->enable("physics_system");
        SANDBOX_LOG_INFO("physics_system enabled after enable: {}", ext_systems->enabled("physics_system"));
    }

    SANDBOX_LOG_INFO("=== Testing extensions::triggers ===");

    auto* ext_triggers = app.get_extension<sandbox::extensions::triggers>("triggers");
    if (ext_triggers)
    {
        ext_triggers->create<position>("on_position_added",
            [](auto& observer_builder) {
                observer_builder.event(flecs::OnSet);
            },
            [](flecs::entity entity, position& pos) {
                SANDBOX_LOG_INFO("trigger fired: entity='{}' position set to x={:.2f} y={:.2f}",
                    entity.name().c_str(), pos.x, pos.y);
            }
        );

        SANDBOX_LOG_INFO("on_position_added trigger exists: {}", ext_triggers->exists("on_position_added"));

        ext_triggers->disable("on_position_added");
        SANDBOX_LOG_INFO("on_position_added enabled after disable: {}", ext_triggers->enabled("on_position_added"));

        ext_triggers->enable("on_position_added");
        SANDBOX_LOG_INFO("on_position_added enabled after enable: {}", ext_triggers->enabled("on_position_added"));
    }

    SANDBOX_LOG_INFO("=== Testing extensions::events ===");

    auto* ext_events = app.get_extension<sandbox::extensions::events>("events");
    if (ext_events)
    {
        ext_events->subscribe<player_spawned_event>("on_player_spawned",
            [](player_spawned_event& event_data) {
                SANDBOX_LOG_INFO("immediate event received: player='{}' at ({:.2f}, {:.2f})",
                    event_data.player_name, event_data.spawn_x, event_data.spawn_y);
            }
        );

        SANDBOX_LOG_INFO("publishing player_spawned_event for player_3");
        ext_events->publish<player_spawned_event>({"player_3", 200.0f, 150.0f});
    }

    SANDBOX_LOG_INFO("=== Running 5 frames to exercise systems ===");

    for (int frame_index = 0; frame_index < 5; ++frame_index)
    {
        app.progress();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));

        auto* player_1_position = ext_storage ? ext_storage->get<position>("player_1") : nullptr;
        if (player_1_position)
        {
            SANDBOX_LOG_DEBUG("frame {}: player_1 position x={:.2f} y={:.2f}", frame_index, player_1_position->x, player_1_position->y);
        }
    }

    SANDBOX_LOG_INFO("=== Verifying final state ===");

    if (ext_stages)
    {
        SANDBOX_LOG_INFO("physics_stage exists: {}", ext_stages->exists("physics_stage"));
        SANDBOX_LOG_INFO("render_stage exists: {}", ext_stages->exists("render_stage"));
    }

    if (ext_storage)
    {
        SANDBOX_LOG_INFO("player_1 exists: {}", ext_storage->exists("player_1"));
        SANDBOX_LOG_INFO("player_2 exists: {}", ext_storage->exists("player_2"));
    }

    if (ext_systems)
    {
        SANDBOX_LOG_INFO("movement_system exists: {}", ext_systems->exists("movement_system"));
    }

    if (ext_triggers)
    {
        SANDBOX_LOG_INFO("on_position_added trigger exists: {}", ext_triggers->exists("on_position_added"));
    }

    if (ext_events)
    {
        SANDBOX_LOG_INFO("on_player_spawned subscriber exists: {}", ext_events->exists("on_player_spawned"));
    }

    SANDBOX_LOG_INFO("=== Extension Tests Complete ===");

    return 0;
}