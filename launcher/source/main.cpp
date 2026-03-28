#include <sandbox/sandbox.h>
#include <thread>
#include <chrono>
#include <filesystem>
#include <string_view>

#include "sandbox/sandbox.h"
#include "sandbox/ecs/scopes_evt.h"
#include "sandbox/ecs/stages_evt.h"
#include "sandbox/ecs/systems_evt.h"
#include "sandbox/ecs/triggers_evt.h"
#include "sandbox/data/storage_evt.h"
#include "sandbox/data/caches_evt.h"
#include "sandbox/system/dependencies_evt.h"
#include "sandbox/io/filesystem_evt.h"
#include "sandbox/diagnostics/logger_evt.h"

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
    const auto test_save_directory = std::filesystem::temp_directory_path() / "sandbox_saves";
    config.set({"serializer", "root_dir"}, test_save_directory.string());

    sandbox::engine app;
    app.initialize(config);

    auto* ext_events = app.get_events();
    if (ext_events)
    {
        ext_events->publish(sandbox::diagnostics::logger_info_evt::create("extensions::logger is working correctly"));
        ext_events->publish(sandbox::diagnostics::logger_warn_evt::create("this is a warning from extensions::logger"));
        ext_events->publish(sandbox::diagnostics::logger_debug_evt::create("debug message from extensions::logger"));

        ext_events->publish(sandbox::ecs::set_scope_evt::create({"game", "world"}));
        ext_events->publish(sandbox::ecs::push_scope_evt::create({"game", "entities"}));
        ext_events->publish(sandbox::ecs::set_scope_evt::create({"game"}));

        ext_events->publish(sandbox::ecs::create_stage_evt::create("physics_stage"));
        ext_events->publish(sandbox::ecs::create_stage_evt::create("render_stage", {"physics_stage"}));
        ext_events->publish(sandbox::ecs::create_stage_evt::create("ai_stage", {"physics_stage"}));

        ext_events->publish(sandbox::data::create_object_evt<>::create("player_1", [](sandbox::extensions::storage& storage_ext, std::string_view name) {
            storage_ext.create<position>(name, 0.0f, 0.0f);
        }));
        ext_events->publish(sandbox::data::create_object_evt<>::create("player_1_vel", [](sandbox::extensions::storage& storage_ext, std::string_view name) {
            storage_ext.create<velocity>(name, 1.0f, 0.5f);
        }));

        ext_events->publish(sandbox::ecs::create_system_evt<>::create("movement_system", "physics_stage",
            [](sandbox::extensions::systems& systems_ext, std::string_view name, std::string_view stage) {
                systems_ext.create<position>(name, stage,
                    [](auto& builder) { builder.interval(0.1f); },
                    [](position& pos) {
                        pos.x += 0.1f;
                        pos.y += 0.05f;
                    });
            }
        ));

        ext_events->publish(sandbox::ecs::create_system_evt<>::create("physics_system", "physics_stage",
            [](sandbox::extensions::systems& systems_ext, std::string_view name, std::string_view stage) {
                systems_ext.create<position, velocity>(name, stage,
                    [](position& pos, const velocity& vel) {
                        pos.x += vel.dx;
                        pos.y += vel.dy;
                    });
            }
        ));

        ext_events->publish(sandbox::ecs::create_trigger_evt<>::create("on_position_added",
            [](sandbox::extensions::triggers& triggers_ext, std::string_view name) {
                triggers_ext.create<position>(name,
                    [](auto& observer_builder) { observer_builder.event(flecs::OnSet); },
                    [](flecs::entity entity, position& pos) {
                        SANDBOX_LOG_INFO("trigger fired: entity='{}' position set to x={:.2f} y={:.2f}",
                            entity.name().c_str(), pos.x, pos.y);
                    });
            }
        ));

        ext_events->subscribe<player_spawned_event>("on_player_spawned",
            [](player_spawned_event& event_data) {
                SANDBOX_LOG_INFO("immediate event received: player='{}' at ({:.2f}, {:.2f})",
                    event_data.player_name, event_data.spawn_x, event_data.spawn_y);
            }
        );

        ext_events->publish<player_spawned_event>({"player_3", 200.0f, 150.0f});

        auto player_entity = app.world.lookup("::objects::player_1");
        ext_events->publish(sandbox::data::save_cache_evt::create(player_entity, 2.0f));
        flecs::entity cached_entity;
        ext_events->publish(sandbox::data::get_cache_evt::create("player_1", &cached_entity));
        SANDBOX_LOG_INFO("cached player_1 exists: {}", cached_entity.is_valid());

        ext_events->publish(sandbox::system::require_dependency_evt::create("logger"));
        ext_events->publish(sandbox::system::require_dependency_evt::create("systems"));
        ext_events->publish(sandbox::system::validate_dependencies_evt::create());
        SANDBOX_LOG_INFO("dependencies ready tag present: {}", app.world.has<sandbox::extensions::dependencies::is_ready_tag>());

        std::string resolved;
        ext_events->publish(sandbox::io::resolve_path_evt::create("res://test/path", &resolved));
        SANDBOX_LOG_INFO("filesystem resolved res://test/path -> {}", resolved);
    }

    // Serializer test: save and load entity
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

        if (ext_events)
        {
            position* player_1_position = nullptr;
            ext_events->publish(sandbox::data::get_object_evt<position>::create("player_1", &player_1_position,
                [](sandbox::extensions::storage& storage_ext, std::string_view name, position** out) {
                    if (out)
                        *out = storage_ext.get<position>(name);
                }));

            if (player_1_position)
            {
                SANDBOX_LOG_DEBUG("frame {}: player_1 position x={:.2f} y={:.2f}", frame_index, player_1_position->x, player_1_position->y);
            }
        }

        auto clock_entity = app.world.lookup("::extensions::clock");
        if (clock_entity.is_valid() && clock_entity.has<sandbox::extensions::clock::state>())
        {
            auto& state = clock_entity.get_mut<sandbox::extensions::clock::state>();
            SANDBOX_LOG_DEBUG("frame {}: clock dt={} total={}", frame_index, state.dt, state.total_time);
        }
    }

    {
        // Load into a new entity to verify
        auto loaded = app.world.entity("loaded_entity");
        sandbox::extensions::request_load(loaded, "serial_entity.json");
        app.progress();

        if (auto* pos = loaded.try_get_mut<position>())
        {
            SANDBOX_LOG_INFO("serializer loaded position x={} y={}", pos->x, pos->y);
        }
    }

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
