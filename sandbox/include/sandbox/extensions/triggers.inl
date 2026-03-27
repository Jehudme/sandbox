#pragma once

#include "logger.h"
#include "sandbox/core/engine.h"

namespace sandbox::extensions
{
    template<typename... components>
    void triggers::create(std::string_view name, auto&& configuration_lambda, auto&& logic_lambda)
    {
        if (!_app) return;

        const std::string absolute_path = "::triggers::" + std::string(name);

        if (auto* log = _app->get_logger())
            log->info("extensions::triggers: creating trigger '{}'", absolute_path);

        auto observer_builder = _app->world.template observer<components...>(absolute_path.c_str());

        if constexpr (std::is_invocable_v<std::remove_reference_t<decltype(configuration_lambda)>, decltype(observer_builder)&>)
        {
            configuration_lambda(observer_builder);
        }

        if constexpr (std::is_invocable_v<std::remove_reference_t<decltype(logic_lambda)>, flecs::iter&>)
        {
            observer_builder.run(std::forward<decltype(logic_lambda)>(logic_lambda));
        }
        else
        {
            observer_builder.each(std::forward<decltype(logic_lambda)>(logic_lambda));
        }

        if (auto* log = _app->get_logger())
            log->debug("extensions::triggers: created trigger '{}'", absolute_path);
    }
}

