#include "sandbox/extensions/scopes.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

namespace sandbox::extensions
{
    void scopes::initialize(sandbox::engine& app, const sandbox::properties& props)
    {
        app._log(sandbox::logger::level::info, "extensions::scopes: initialized");
    }

    void scopes::finalize(sandbox::engine& app)
    {
        app._log(sandbox::logger::level::info, "extensions::scopes: finalized");
    }

    void scopes::set_scope(const scope_path& desired_path)
    {
        if (!_app)
        {
            return;
        }

        if (desired_path.empty())
        {
            _app->_log(sandbox::logger::level::debug, "extensions::scopes: set_scope to root");
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

        _app->_log(sandbox::logger::level::debug, "extensions::scopes: set_scope '{}'", full_path);
        _app->world.set_scope(_app->world.entity(full_path.c_str()));
    }

    void scopes::push_scope(const scope_path& desired_path)
    {
        if (!_app)
        {
            return;
        }

        if (desired_path.empty())
        {
            _app->_log(sandbox::logger::level::debug, "extensions::scopes: push_scope to root");
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

        _app->_log(sandbox::logger::level::debug, "extensions::scopes: push_scope '{}'", full_path);
        _app->world.set_scope(_app->world.entity(full_path.c_str()));
    }
}
