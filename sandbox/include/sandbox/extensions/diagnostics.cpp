#include "diagnostics.h"

#include "sandbox/extensions/logger.h"
#include "sandbox/extensions/systems.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

#include <flecs/addons/stats.h>

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
                auto states = it.field<state>(0);
                auto* log = _app ? _app->get_logger() : nullptr;

                ecs_world_stats_t world_stats{};
                ecs_world_stats_get(it.world().c_ptr(), &world_stats);
                ecs_world_stats_reduce(&world_stats, &world_stats);

                const int sample_index = world_stats.t;

                for (auto row : it)
                {
                    auto& diag_state = states[row];
                    diag_state.fps = world_stats.performance.fps.gauge.avg[sample_index];

                    ecs_query_desc_t desc{};
                    desc.terms[0].id = flecs::_::type<profile_tag>::id(it.world());
                    ecs_query_t* profile_query = ecs_query_init(it.world().c_ptr(), &desc);

                    ecs_iter_t qit = ecs_query_iter(it.world().c_ptr(), profile_query);
                    while (ecs_query_next(&qit))
                    {
                        for (int i = 0; i < qit.count; ++i)
                        {
                            flecs::entity system_entity = flecs::entity(it.world(), qit.entities[i]);
                            ecs_system_stats_t sys_stats{};
                            if (ecs_system_stats_get(qit.world, system_entity.id(), &sys_stats))
                            {
                                const float last_ms = sys_stats.time_spent.gauge.avg[sample_index] * 1000.0f;
                                std::string system_name = system_entity.path().c_str();
                                auto& entry = diag_state.system_times[system_name.empty() ? "<unnamed>" : system_name];
                                entry.last_ms = last_ms;
                                entry.avg_ms = entry.avg_ms > 0.0f ? (entry.avg_ms * 0.9f + last_ms * 0.1f) : last_ms;
                            }
                        }
                    }
                    ecs_query_fini(profile_query);
                }

                if (log) log->debug("extensions::diagnostics: sampled fps={}", world_stats.performance.fps.gauge.avg[sample_index]);
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
