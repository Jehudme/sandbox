#include "sandbox/diagnostics/diagnostics_ext.h"
#include "sandbox/diagnostics/logger.h"
#include "sandbox/ecs/systems_ext.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/properties.h"

namespace sandbox::extensions
{
    void diagnostics::initialize(const sandbox::properties& properties)
    {
        if (!_app) return;

        auto* log = _app->get_logger();
        _app->world.component<metric>();
        _app->world.component<state>();
        _app->world.component<profile_tag>();

        if (auto diag_entity = _app->world.lookup("::extensions::diagnostics"); diag_entity.is_valid())
        {
            diag_entity.set<state>({});
        }

        auto* ext_systems = _app->get_extension<extensions::systems>("systems");
        if (!ext_systems)
        {
            if (log) log->error("extensions::diagnostics: systems extension unavailable");
            return;
        }

        ext_systems->create<state>(
            "diagnostics_sampler",
            "",
            [](auto& builder) { builder.kind(flecs::PostUpdate); },
            [this](flecs::iter& it)
            {
                auto* log = _app ? _app->get_logger() : nullptr;
                while (it.next())
                {
                    auto states = it.field<state>(0);
                    const auto* world_info = ecs_get_world_info(it.world().c_ptr());
                    const float frame_dt = world_info ? static_cast<float>(world_info->delta_time) : 0.0f;
                    const float frame_ms = frame_dt * 1000.0f;
                    const float fps = frame_dt > 0.0f ? (1.0f / frame_dt) : 0.0f;
                    const float entity_count = static_cast<float>(it.world().count(flecs::Wildcard));

                    for (auto row : it)
                    {
                        auto& diag_state = states[row];
                        diag_state.fps = fps;
                        auto& frame_metric = diag_state.system_times["__frame__"];
                        frame_metric.last_ms = frame_ms;
                        frame_metric.avg_ms = frame_metric.avg_ms > 0.0f
                            ? (frame_metric.avg_ms * 0.9f + frame_ms * 0.1f)
                            : frame_ms;

                        auto& entities_metric = diag_state.system_times["__entities__"];
                        entities_metric.last_ms = entity_count;
                        entities_metric.avg_ms = entities_metric.avg_ms > 0.0f
                            ? (entities_metric.avg_ms * 0.9f + entity_count * 0.1f)
                            : entity_count;
                    }

                    if (log) log->debug("extensions::diagnostics: sampled fps={}", fps);
                }
            }
        );

        if (log) log->info("extensions::diagnostics: initialized");
    }

    void diagnostics::finalize()
    {
        if (!_app) return;

        if (auto* ext_systems = _app->get_extension<extensions::systems>("systems"))
        {
            ext_systems->destroy("diagnostics_sampler");
        }

        auto diag_entity = _app->world.lookup("::extensions::diagnostics");
        if (diag_entity.is_valid() && diag_entity.has<state>())
        {
            diag_entity.remove<state>();
        }
    }
}
