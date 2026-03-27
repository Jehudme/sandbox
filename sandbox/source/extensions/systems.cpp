#include "sandbox/extensions/systems.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

namespace sandbox::extensions
{
    void systems::initialize(const sandbox::properties& props)
    {
    }

    void systems::finalize()
    {
    }

    void systems::destroy(std::string_view name)
    {

        const std::string absolute_path = "::systems::" + std::string(name);
        _app->get_logger()->info("extensions::systems: destroying system '{}'", absolute_path);

        auto system_entity = _app->world.lookup(absolute_path.c_str());
        if (!system_entity.is_valid())
        {
            _app->get_logger()->warn("extensions::systems: system '{}' not found", absolute_path);
            return;
        }

        system_entity.destruct();
    }

    void systems::enable(std::string_view name)
    {

        const std::string absolute_path = "::systems::" + std::string(name);
        auto system_entity = _app->world.lookup(absolute_path.c_str());

        if (!system_entity.is_valid())
        {
            _app->get_logger()->warn("extensions::systems: system '{}' not found for enable", absolute_path);
            return;
        }

        system_entity.enable();
        _app->get_logger()->debug("extensions::systems: enabled system '{}'", absolute_path);
    }

    void systems::disable(std::string_view name)
    {

        const std::string absolute_path = "::systems::" + std::string(name);
        auto system_entity = _app->world.lookup(absolute_path.c_str());

        if (!system_entity.is_valid())
        {
            _app->get_logger()->warn("extensions::systems: system '{}' not found for disable", absolute_path);
            return;
        }

        system_entity.disable();
        _app->get_logger()->debug("extensions::systems: disabled system '{}'", absolute_path);
    }

    bool systems::exists(std::string_view name) const
    {


        const std::string absolute_path = "::systems::" + std::string(name);
        return _app->world.lookup(absolute_path.c_str()).is_valid();
    }

    bool systems::enabled(std::string_view name) const
    {


        const std::string absolute_path = "::systems::" + std::string(name);
        auto system_entity = _app->world.lookup(absolute_path.c_str());
        return system_entity.is_valid() && system_entity.enabled();
    }
}