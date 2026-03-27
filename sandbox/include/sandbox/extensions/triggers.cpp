#include "sandbox/extensions/triggers.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

namespace sandbox::extensions
{
    void triggers::initialize(sandbox::engine& app, const sandbox::properties& props)
    {
        app._log(sandbox::logger::level::info, "extensions::triggers: initialized");
    }

    void triggers::finalize(sandbox::engine& app)
    {
        app._log(sandbox::logger::level::info, "extensions::triggers: finalized");
    }

    void triggers::destroy(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::triggers::" + std::string(name);
        _app->_log(sandbox::logger::level::info, "extensions::triggers: destroying trigger '{}'", absolute_path);

        auto trigger_entity = _app->world.lookup(absolute_path.c_str());
        if (!trigger_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::triggers: trigger '{}' not found", absolute_path);
            return;
        }

        trigger_entity.destruct();
    }

    void triggers::enable(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::triggers::" + std::string(name);
        auto trigger_entity = _app->world.lookup(absolute_path.c_str());

        if (!trigger_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::triggers: trigger '{}' not found for enable", absolute_path);
            return;
        }

        trigger_entity.enable();
        _app->_log(sandbox::logger::level::debug, "extensions::triggers: enabled trigger '{}'", absolute_path);
    }

    void triggers::disable(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::triggers::" + std::string(name);
        auto trigger_entity = _app->world.lookup(absolute_path.c_str());

        if (!trigger_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::triggers: trigger '{}' not found for disable", absolute_path);
            return;
        }

        trigger_entity.disable();
        _app->_log(sandbox::logger::level::debug, "extensions::triggers: disabled trigger '{}'", absolute_path);
    }

    bool triggers::exists(std::string_view name) const
    {
        if (!_app)
        {
            return false;
        }

        const std::string absolute_path = "::triggers::" + std::string(name);
        return _app->world.lookup(absolute_path.c_str()).is_valid();
    }

    bool triggers::enabled(std::string_view name) const
    {
        if (!_app)
        {
            return false;
        }

        const std::string absolute_path = "::triggers::" + std::string(name);
        auto trigger_entity = _app->world.lookup(absolute_path.c_str());
        return trigger_entity.is_valid() && trigger_entity.enabled();
    }
}