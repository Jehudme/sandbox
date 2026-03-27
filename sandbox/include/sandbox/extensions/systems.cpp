#include "sandbox/extensions/systems.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

namespace sandbox::extensions
{
    void systems::initialize(sandbox::engine& app, const sandbox::properties& props)
    {
        app._log(sandbox::logger::level::info, "extensions::systems: initialized");
    }

    void systems::finalize(sandbox::engine& app)
    {
        app._log(sandbox::logger::level::info, "extensions::systems: finalized");
    }

    void systems::destroy(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::systems::" + std::string(name);
        _app->_log(sandbox::logger::level::info, "extensions::systems: destroying system '{}'", absolute_path);

        auto system_entity = _app->world.lookup(absolute_path.c_str());
        if (!system_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::systems: system '{}' not found", absolute_path);
            return;
        }

        system_entity.destruct();
    }

    void systems::enable(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::systems::" + std::string(name);
        auto system_entity = _app->world.lookup(absolute_path.c_str());

        if (!system_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::systems: system '{}' not found for enable", absolute_path);
            return;
        }

        system_entity.enable();
        _app->_log(sandbox::logger::level::debug, "extensions::systems: enabled system '{}'", absolute_path);
    }

    void systems::disable(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::systems::" + std::string(name);
        auto system_entity = _app->world.lookup(absolute_path.c_str());

        if (!system_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::systems: system '{}' not found for disable", absolute_path);
            return;
        }

        system_entity.disable();
        _app->_log(sandbox::logger::level::debug, "extensions::systems: disabled system '{}'", absolute_path);
    }

    bool systems::exists(std::string_view name) const
    {
        if (!_app)
        {
            return false;
        }

        const std::string absolute_path = "::systems::" + std::string(name);
        return _app->world.lookup(absolute_path.c_str()).is_valid();
    }

    bool systems::enabled(std::string_view name) const
    {
        if (!_app)
        {
            return false;
        }

        const std::string absolute_path = "::systems::" + std::string(name);
        auto system_entity = _app->world.lookup(absolute_path.c_str());
        return system_entity.is_valid() && system_entity.enabled();
    }
}