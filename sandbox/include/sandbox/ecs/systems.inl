#pragma once

#include "sandbox/diagnostics/logger.h"
#include "sandbox/core/engine.h"

namespace sandbox::extensions
{
    template<typename... components>
    void systems::create(std::string_view name, std::string_view stage, auto&& configuration_lambda, auto&& logic_lambda)
    {
        if (!_app) return;

        const std::string absolute_path = "::systems::" + std::string(name);

        if (auto* log = _app->get_logger())
            log->info("extensions::systems: creating system '{}'", absolute_path);

        auto system_builder = _app->world.template system<components...>(absolute_path.c_str());

        if (!stage.empty())
        {
            // starts_with requires C++20; this project targets C++23 — confirmed safe.
            const std::string stage_path = stage.starts_with("::") ? std::string(stage) : "::stages::" + std::string(stage);
            auto stage_entity = _app->world.lookup(stage_path.c_str());

            if (!stage_entity.is_valid())
            {
                if (auto* log = _app->get_logger())
                    log->warn("extensions::systems: stage '{}' not found for system '{}'", stage_path, absolute_path);
            }
            else
            {
                system_builder.kind(stage_entity);
            }
        }

        configuration_lambda(system_builder);

        if constexpr (std::is_invocable_v<std::remove_reference_t<decltype(logic_lambda)>, flecs::iter&>)
        {
            system_builder.run(std::forward<decltype(logic_lambda)>(logic_lambda));
        }
        else
        {
            system_builder.each(std::forward<decltype(logic_lambda)>(logic_lambda));
        }

        if (auto* log = _app->get_logger())
            log->debug("extensions::systems: created system '{}'", absolute_path);
    }

    template<typename... components>
    void systems::create(std::string_view name, std::string_view stage, auto&& logic_lambda)
    {
        create<components...>(name, stage, [](auto&) {}, std::forward<decltype(logic_lambda)>(logic_lambda));
    }
}

