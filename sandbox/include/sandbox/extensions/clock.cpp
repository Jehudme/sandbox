#include "clock.h"

#include "sandbox/extensions/logger.h"
#include "sandbox/extensions/systems.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

#include <algorithm>

namespace sandbox::extensions
{
    namespace
    {
        constexpr float kDefaultFixedDt = 1.0f / 60.0f;

        float compute_safe_delta_time(const flecs::iter& it, const ecs_world_info_t* info)
        {
            const float world_dt = info ? static_cast<float>(info->delta_time) : 0.0f;
            const float iter_dt  = static_cast<float>(it.delta_time());
            return iter_dt > 0.0f ? iter_dt : world_dt;
        }

        void process_time_scale_requests(flecs::world world, clock::state& data, float dt, sandbox::extensions::logger* log)
        {
            auto request_query = world.query_builder<clock::time_scale_request>().build();

            request_query.each(
                [&](flecs::entity e, clock::time_scale_request& request)
                {
                    const float delta_scale = request.lerp_speed * dt;
                    const float difference  = request.target_scale - data.time_scale;

                    if (std::abs(difference) <= delta_scale)
                    {
                        data.time_scale = request.target_scale;
                        if (log) log->debug("extensions::clock: time scale snapped to target={}", request.target_scale);
                        e.remove<clock::time_scale_request>();
                    }
                    else
                    {
                        data.time_scale += (difference > 0.0f ? delta_scale : -delta_scale);
                        if (log) log->debug("extensions::clock: time scale adjusted to {}", data.time_scale);
                    }
                });
        }
    }

    void clock::initialize(const sandbox::properties& props)
    {
        if (!_app) return;

        auto* log = _app->get_logger();

        _app->world.component<state>();
        _app->world.component<fixed_update_tag>();
        _app->world.component<time_scale_request>();

        state initial {
            .dt          = 0.0f,
            .fixed_dt    = props.get<float>({"fixed_dt"}).value_or(kDefaultFixedDt),
            .total_time  = 0.0f,
            .time_scale  = props.get<float>({"time_scale"}).value_or(1.0f),
            .accumulator = 0.0f
        };

        if (initial.fixed_dt <= 0.0f)
        {
            initial.fixed_dt = kDefaultFixedDt;
            if (log) log->warn("extensions::clock: invalid fixed_dt provided; defaulting to 1/60s");
        }

        if (auto clock_entity = _app->world.lookup("::extensions::clock"); clock_entity.is_valid())
        {
            clock_entity.set<state>(initial);
        }

        auto* ext_systems = _app->get_extension<extensions::systems>("systems");
        if (!ext_systems)
        {
            if (log) log->error("extensions::clock: systems extension unavailable; cannot create tick system");
            return;
        }

        ext_systems->create<state>(
            "clock_tick",
            "",
            [](auto& builder) { builder.kind(flecs::PreUpdate); },
            [this](flecs::iter& it)
            {
                auto states = it.field<state>(0);
                auto* log = _app ? _app->get_logger() : nullptr;
                auto* info = ecs_get_world_info(it.world());
                for (auto row : it)
                {
                    auto& data = states[row];
                    const float raw_dt = compute_safe_delta_time(it, info);

                    process_time_scale_requests(it.world(), data, raw_dt, log);

                    data.dt = raw_dt * data.time_scale;
                    data.total_time += data.dt;
                    data.accumulator += data.dt;

                    // Emit as many fixed pulses as needed to catch up
                    constexpr int kMaxPulsesPerFrame = 8;
                    int pulse_count = 0;
                    while (data.accumulator >= data.fixed_dt && pulse_count < kMaxPulsesPerFrame)
                    {
                        data.accumulator -= data.fixed_dt;
                        it.world().event<fixed_update_tag>().id<fixed_update_tag>().emit();
                        ++pulse_count;
                    }

                    if (pulse_count >= kMaxPulsesPerFrame && data.accumulator >= data.fixed_dt)
                    {
                        // Avoid spiral-of-death by discarding excessive backlog.
                        data.accumulator = 0.0f;
                        if (log) log->warn("extensions::clock: fixed-update pulses clamped to {} in one frame", kMaxPulsesPerFrame);
                    }

                    if (log && pulse_count > 0)
                    {
                        log->debug("extensions::clock: emitted {} fixed pulses (dt={}, fixed_dt={})",
                                   pulse_count, data.dt, data.fixed_dt);
                    }
                }
            }
        );

        if (log) log->info("extensions::clock: initialized");
    }

    void clock::finalize()
    {
        if (!_app) return;

        auto* log = _app->get_logger();
        if (auto* ext_systems = _app->get_extension<extensions::systems>("systems"))
        {
            ext_systems->destroy("clock_tick");
        }

        _app->world.each<time_scale_request>([](flecs::entity e, time_scale_request&)
        {
            e.remove<time_scale_request>();
        });

        if (auto clock_entity = _app->world.lookup("::extensions::clock"); clock_entity.is_valid())
        {
            if (clock_entity.has<state>())
            {
                clock_entity.remove<state>();
            }
        }

        if (log) log->info("extensions::clock: finalized");
    }
}
