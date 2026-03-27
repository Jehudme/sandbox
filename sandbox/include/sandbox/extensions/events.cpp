#include "sandbox/extensions/events.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

namespace sandbox::extensions
{
    void events::initialize(sandbox::engine& app, const sandbox::properties& props)
    {
        app._log(sandbox::logger::level::info, "extensions::events: initialized");
    }

    void events::finalize(sandbox::engine& app)
    {
        app._log(sandbox::logger::level::info, "extensions::events: finalized");
    }

    void events::destroy(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::events::" + std::string(name);
        _app->_log(sandbox::logger::level::info, "extensions::events: destroying subscriber '{}'", absolute_path);

        auto subscriber_entity = _app->world.lookup(absolute_path.c_str());
        if (!subscriber_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::events: subscriber '{}' not found", absolute_path);
            return;
        }

        subscriber_entity.destruct();
    }

    void events::enable(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::events::" + std::string(name);
        auto subscriber_entity = _app->world.lookup(absolute_path.c_str());

        if (!subscriber_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::events: subscriber '{}' not found for enable", absolute_path);
            return;
        }

        subscriber_entity.enable();
        _app->_log(sandbox::logger::level::debug, "extensions::events: enabled subscriber '{}'", absolute_path);
    }

    void events::disable(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::events::" + std::string(name);
        auto subscriber_entity = _app->world.lookup(absolute_path.c_str());

        if (!subscriber_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::events: subscriber '{}' not found for disable", absolute_path);
            return;
        }

        subscriber_entity.disable();
        _app->_log(sandbox::logger::level::debug, "extensions::events: disabled subscriber '{}'", absolute_path);
    }

    bool events::exists(std::string_view name) const
    {
        if (!_app)
        {
            return false;
        }

        const std::string absolute_path = "::events::" + std::string(name);
        return _app->world.lookup(absolute_path.c_str()).is_valid();
    }

    bool events::enabled(std::string_view name) const
    {
        if (!_app)
        {
            return false;
        }

        const std::string absolute_path = "::events::" + std::string(name);
        auto subscriber_entity = _app->world.lookup(absolute_path.c_str());
        return subscriber_entity.is_valid() && subscriber_entity.enabled();
    }
}