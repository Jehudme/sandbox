#include "sandbox/extensions/stages.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"

namespace sandbox::extensions
{
    void stages::initialize(sandbox::engine& app, const sandbox::properties& props)
    {
        app._log(sandbox::logger::level::info, "extensions::stages: initialized");
    }

    void stages::finalize(sandbox::engine& app)
    {
        app._log(sandbox::logger::level::info, "extensions::stages: finalized");
    }

    void stages::create(std::string_view name, const dependencies& stage_dependencies)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::stages::" + std::string(name);
        _app->_log(sandbox::logger::level::info, "extensions::stages: creating stage '{}'", absolute_path);

        auto stage_entity = _app->world.entity(absolute_path.c_str()).add(flecs::Phase);

        for (const auto& dependency_name : stage_dependencies)
        {
            const std::string dependency_path = "::stages::" + std::string(dependency_name);
            auto dependency_entity = _app->world.lookup(dependency_path.c_str());

            if (!dependency_entity.is_valid())
            {
                _app->_log(sandbox::logger::level::warn, "extensions::stages: dependency '{}' not found for stage '{}'", dependency_path, absolute_path);
                continue;
            }

            stage_entity.depends_on(dependency_entity);
            _app->_log(sandbox::logger::level::debug, "extensions::stages: stage '{}' depends_on '{}'", absolute_path, dependency_path);
        }
    }

    void stages::destroy(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::stages::" + std::string(name);
        _app->_log(sandbox::logger::level::info, "extensions::stages: destroying stage '{}'", absolute_path);

        auto stage_entity = _app->world.lookup(absolute_path.c_str());
        if (!stage_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::stages: stage '{}' not found", absolute_path);
            return;
        }

        stage_entity.destruct();
    }

    void stages::enable(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::stages::" + std::string(name);
        auto stage_entity = _app->world.lookup(absolute_path.c_str());

        if (!stage_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::stages: stage '{}' not found for enable", absolute_path);
            return;
        }

        stage_entity.enable();
        _app->_log(sandbox::logger::level::debug, "extensions::stages: enabled stage '{}'", absolute_path);
    }

    void stages::disable(std::string_view name)
    {
        if (!_app)
        {
            return;
        }

        const std::string absolute_path = "::stages::" + std::string(name);
        auto stage_entity = _app->world.lookup(absolute_path.c_str());

        if (!stage_entity.is_valid())
        {
            _app->_log(sandbox::logger::level::warn, "extensions::stages: stage '{}' not found for disable", absolute_path);
            return;
        }

        stage_entity.disable();
        _app->_log(sandbox::logger::level::debug, "extensions::stages: disabled stage '{}'", absolute_path);
    }

    bool stages::exists(std::string_view name) const
    {
        if (!_app)
        {
            return false;
        }

        const std::string absolute_path = "::stages::" + std::string(name);
        return _app->world.lookup(absolute_path.c_str()).is_valid();
    }

    bool stages::enabled(std::string_view name) const
    {
        if (!_app)
        {
            return false;
        }

        const std::string absolute_path = "::stages::" + std::string(name);
        auto stage_entity = _app->world.lookup(absolute_path.c_str());
        return stage_entity.is_valid() && stage_entity.enabled();
    }
}