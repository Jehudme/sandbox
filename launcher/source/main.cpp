#include <sandbox/sandbox.h>
#include <thread>
#include <chrono>
#include <filesystem>

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

RTTR_REGISTRATION
{
    rttr::registration::class_<position>("position")
        .property("x", &position::x)
        .property("y", &position::y);

    rttr::registration::class_<velocity>("velocity")
        .property("dx", &velocity::dx)
        .property("dy", &velocity::dy);
}

int main()
{
    SANDBOX_LOG_INFO("=== Starting Extension Tests ===");

    sandbox::properties config;
    config.set({"logger", "level"}, std::string("TRACE"));
    config.set({"serializer", "root_dir"}, std::string("/tmp/sandbox_saves"));

    sandbox::engine app;
    app.initialize(config);

    auto* ext_logger = app.get_logger();
    if (ext_logger)
    {
        ext_logger->info("extensions::logger is working correctly");
        ext_logger->warn("this is a warning from extensions::logger");
        ext_logger->debug("debug message from extensions::logger");
    }

    auto* ext_scopes = app.get_scopes();
    if (ext_scopes)
    {
        ext_scopes->set_scope({"game", "world"});
        ext_scopes->push_scope({"game", "entities"});
        ext_scopes->set_scope({"game"});
    }

    auto* ext_stages = app.get_extension<sandbox::extensions::stages>("stages");
    if (ext_stages)
    {
        ext_stages->create("physics_stage");
        ext_stages->create("render_stage", {"physics_stage"});
        ext_stages->create("ai_stage", {"physics_stage"});
    }

    auto* ext_storage = app.get_storage();
    auto* ext_systems = app.get_systems();
    auto* ext_triggers = app.get_triggers();
    auto* ext_events = app.get_events();
    auto* ext_caches = app.get_caches();
    auto* ext_clock = app.get_clock();
    auto* ext_filesystem = app.get_filesystem();
    auto* ext_dependencies = app.get_dependencies();
    auto* ext_serializer = app.get_serializer();
    auto* ext_diagnostics = app.get_diagnostics();

    if (ext_storage)
    {
        ext_storage->create<position>("player_1", 0.0f, 0.0f);
        ext_storage->create<velocity>("player_1_vel", 1.0f, 0.5f);
    }

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
    }

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
    }

    if (ext_events)
    {
        ext_events->subscribe<player_spawned_event>("on_player_spawned",
            [](player_spawned_event& event_data) {
                SANDBOX_LOG_INFO("immediate event received: player='{}' at ({:.2f}, {:.2f})",
                    event_data.player_name, event_data.spawn_x, event_data.spawn_y);
            }
        );

        ext_events->publish<player_spawned_event>({"player_3", 200.0f, 150.0f});
    }

    if (ext_caches)
    {
        auto player_entity = app.world.lookup("::objects::player_1");
        ext_caches->save(player_entity, 2.0f);
        SANDBOX_LOG_INFO("cached player_1 exists: {}", ext_caches->get("player_1").is_valid());
    }

    if (ext_dependencies)
    {
        ext_dependencies->require("logger");
        ext_dependencies->require("systems");
        ext_dependencies->validate();
        SANDBOX_LOG_INFO("dependencies ready tag present: {}", app.world.has<sandbox::extensions::dependencies::is_ready_tag>());
    }

    if (ext_filesystem)
    {
        auto resolved = ext_filesystem->resolve("res://test/path");
        SANDBOX_LOG_INFO("filesystem resolved res://test/path -> {}", resolved);
    }

    // Serializer test: save and load entity
    if (ext_serializer)
    {
        auto entity = app.world.entity("serial_entity");
        entity.set<position>({10.0f, 20.0f});
        entity.set<velocity>({1.0f, -1.0f});
        sandbox::extensions::request_save(entity, "serial_entity.json");
    }

    // Mark movement system for diagnostics
    auto movement_system_entity = app.world.lookup("::systems::movement_system");
    if (movement_system_entity.is_valid())
    {
        movement_system_entity.add<sandbox::extensions::diagnostics::profile_tag>();
    }

    SANDBOX_LOG_INFO("=== Running 6 frames to exercise systems and extensions ===");

    for (int frame_index = 0; frame_index < 6; ++frame_index)
    {
        app.progress();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));

        if (ext_storage)
        {
            if (auto* player_1_position = ext_storage->get<position>("player_1"))
            {
                SANDBOX_LOG_DEBUG("frame {}: player_1 position x={:.2f} y={:.2f}", frame_index, player_1_position->x, player_1_position->y);
            }
        }

        if (ext_clock)
        {
            auto clock_entity = app.world.lookup("::extensions::clock");
            if (clock_entity.is_valid() && clock_entity.has<sandbox::extensions::clock::state>())
            {
                auto& state = clock_entity.get_mut<sandbox::extensions::clock::state>();
                SANDBOX_LOG_DEBUG("frame {}: clock dt={} total={}", frame_index, state.dt, state.total_time);
            }
        }
    }

    if (ext_serializer)
    {
        // Load into a new entity to verify
        auto loaded = app.world.entity("loaded_entity");
        sandbox::extensions::request_load(loaded, "serial_entity.json");
        app.progress();

        if (auto* pos = loaded.get_mut<position>())
        {
            SANDBOX_LOG_INFO("serializer loaded position x={} y={}", pos->x, pos->y);
        }
    }

    if (ext_diagnostics)
    {
        auto diag_entity = app.world.lookup("::extensions::diagnostics");
        if (diag_entity.is_valid() && diag_entity.has<sandbox::extensions::diagnostics::state>())
        {
            auto& diag_state = diag_entity.get_mut<sandbox::extensions::diagnostics::state>();
            SANDBOX_LOG_INFO("diagnostics fps={}", diag_state.fps);
            for (const auto& [name, metric] : diag_state.system_times)
            {
                SANDBOX_LOG_INFO("system '{}' last_ms={} avg_ms={}", name, metric.last_ms, metric.avg_ms);
            }
        }
    }

    SANDBOX_LOG_INFO("=== Extension Tests Complete ===");
    return 0;
}
