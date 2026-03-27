#include "sandbox/extensions/scopes.h"

#include "../../include/sandbox/extensions/logger.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

namespace sandbox::extensions
{
    void scopes::initialize(const sandbox::properties& props)
    {
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
