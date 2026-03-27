#include "sandbox/system/dependencies.h"
#include "sandbox/diagnostics/logger.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/properties.h"

#include <algorithm>

namespace sandbox::extensions
{
    namespace
    {
        dependencies::state& get_or_create_state(flecs::world world, const char* entity_path)
        {
            auto dep_entity = world.lookup(entity_path);
            if (dep_entity.is_valid())
            {
                return dep_entity.get_mut<dependencies::state>();
            }

            // Fallback to singleton storage on the world
            return world.get_mut<dependencies::state>();
        }
    }

    void dependencies::initialize(const sandbox::properties& properties)
    {
        if (!_app) return;

        auto* log = _app->get_logger();
        _app->world.component<state>();
        _app->world.component<is_ready_tag>();

        state initial{};
        if (auto configured = properties.get<std::vector<std::string>>({"requirements"}))
        {
            initial.requirements = *configured;
        }

        if (auto dep_entity = _app->world.lookup("::extensions::dependencies"); dep_entity.is_valid())
        {
            dep_entity.set<state>(initial);
        }
        else
        {
            _app->world.set<state>(initial);
        }

        validate();

        if (log) log->info("extensions::dependencies: initialized with {} requirement(s)", initial.requirements.size());
    }

    void dependencies::finalize()
    {
        if (!_app) return;
        _app->world.remove<is_ready_tag>();

        if (auto dep_entity = _app->world.lookup("::extensions::dependencies"); dep_entity.is_valid() && dep_entity.has<state>())
        {
            dep_entity.remove<state>();
        }
    }

    void dependencies::require(std::string_view extension_name)
    {
        if (!_app) return;

        auto& dep_state = get_or_create_state(_app->world, "::extensions::dependencies");

        if (std::find(dep_state.requirements.begin(), dep_state.requirements.end(), extension_name) == dep_state.requirements.end())
        {
            dep_state.requirements.emplace_back(extension_name);
        }

        validate();
    }

    void dependencies::validate()
    {
        if (!_app) return;

        auto* log = _app->get_logger();
        auto& dep_state = get_or_create_state(_app->world, "::extensions::dependencies");
        bool all_present = true;
        for (const auto& requirement : dep_state.requirements)
        {
            const std::string absolute_path = "::extensions::" + requirement;
            auto ext_entity = _app->world.lookup(absolute_path.c_str());
            const bool available = ext_entity.is_valid()
                && ext_entity.has<std::unique_ptr<sandbox::extension>>();

            if (!available)
            {
                all_present = false;
                if (log) log->warn("extensions::dependencies: missing required extension '{}'", requirement);
            }
        }

        if (all_present)
        {
            _app->world.add<is_ready_tag>();
            if (log) log->info("extensions::dependencies: all requirements satisfied");
        }
        else
        {
            _app->world.remove<is_ready_tag>();
        }
    }
}
