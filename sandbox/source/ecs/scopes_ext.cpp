#include "sandbox/ecs/scopes_ext.h"
#include "sandbox/ecs/scopes_evt.h"
#include "sandbox/ecs/events_ext.h"
#include "sandbox/diagnostics/logger.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/properties.h"
#include <vector>

namespace sandbox::extensions
{
    void scopes::initialize(const sandbox::properties& props)
    {
        if (auto* ext_events = _app->get_extension<extensions::events>("events"))
        {
            ext_events->subscribe<sandbox::ecs::set_scope_evt>("scopes_set_scope", [this](sandbox::ecs::set_scope_evt& e) {
                std::vector<std::string_view> path_views;
                path_views.reserve(e.path.size());
                for (const auto& segment : e.path)
                {
                    path_views.push_back(segment);
                }
                this->set_scope(path_views);
            });

            ext_events->subscribe<sandbox::ecs::push_scope_evt>("scopes_push_scope", [this](sandbox::ecs::push_scope_evt& e) {
                std::vector<std::string_view> path_views;
                path_views.reserve(e.path.size());
                for (const auto& segment : e.path)
                {
                    path_views.push_back(segment);
                }
                this->push_scope(path_views);
            });
        }
    }

    void scopes::finalize()
    {
    }

    void scopes::set_scope(const scope_path& desired_path)
    {

        if (desired_path.empty())
        {
            _app->get_logger()->debug("extensions::scopes: set_scope to root");
            _app->world.set_scope(flecs::entity());
            return;
        }

        std::string full_path = "::";
        for (size_t index = 0; index < desired_path.size(); ++index)
        {
            full_path += desired_path[index];
            if (index < desired_path.size() - 1)
            {
                full_path += "::";
            }
        }

        _app->get_logger()->debug("extensions::scopes: set_scope '{}'", full_path);
        _app->world.set_scope(_app->world.entity(full_path.c_str()));
    }

    void scopes::push_scope(const scope_path& desired_path)
    {

        if (desired_path.empty())
        {
            _app->get_logger()->debug("extensions::scopes: push_scope to root");
            _app->world.set_scope(flecs::entity());
            return;
        }

        std::string full_path = "::";
        for (size_t index = 0; index < desired_path.size(); ++index)
        {
            full_path += desired_path[index];
            if (index < desired_path.size() - 1)
            {
                full_path += "::";
            }
        }

        _app->get_logger()->debug("extensions::scopes: push_scope '{}'", full_path);
        _app->world.set_scope(_app->world.entity(full_path.c_str()));
    }
}
