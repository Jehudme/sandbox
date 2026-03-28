#include "sandbox/ecs/stages_ext.h"
#include "sandbox/ecs/stages_evt.h"
#include "sandbox/ecs/events_ext.h"
#include "sandbox/diagnostics/logger.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/properties.h"

namespace sandbox::extensions
{
    void stages::initialize(const sandbox::properties& props)
    {
        if (auto* ext_events = _app->get_extension<extensions::events>("events"))
        {
            ext_events->subscribe<sandbox::ecs::create_stage_evt>("stages_create", [this](sandbox::ecs::create_stage_evt& e) {
                std::vector<std::string_view> deps;
                deps.reserve(e.dependencies.size());
                for (const auto& dep : e.dependencies)
                {
                    deps.push_back(dep);
                }
                this->create(e.name, deps);
            });

            ext_events->subscribe<sandbox::ecs::destroy_stage_evt>("stages_destroy", [this](sandbox::ecs::destroy_stage_evt& e) {
                this->destroy(e.name);
            });

            ext_events->subscribe<sandbox::ecs::enable_stage_evt>("stages_enable", [this](sandbox::ecs::enable_stage_evt& e) {
                this->enable(e.name);
            });

            ext_events->subscribe<sandbox::ecs::disable_stage_evt>("stages_disable", [this](sandbox::ecs::disable_stage_evt& e) {
                this->disable(e.name);
            });

            ext_events->subscribe<sandbox::ecs::stage_exists_evt>("stages_exists", [this](sandbox::ecs::stage_exists_evt& e) {
                if (e.out_exists)
                    *e.out_exists = this->exists(e.name);
            });

            ext_events->subscribe<sandbox::ecs::stage_enabled_evt>("stages_enabled", [this](sandbox::ecs::stage_enabled_evt& e) {
                if (e.out_enabled)
                    *e.out_enabled = this->enabled(e.name);
            });
        }
    }

    void stages::finalize()
    {
    }

    void stages::create(std::string_view name, const dependencies& stage_dependencies)
    {

        const std::string absolute_path = "::stages::" + std::string(name);
        _app->get_logger()->info("extensions::stages: creating stage '{}'", absolute_path);

        auto stage_entity = _app->world.entity(absolute_path.c_str()).add(flecs::Phase);

        for (const auto& dependency_name : stage_dependencies)
        {
            const std::string dependency_path = "::stages::" + std::string(dependency_name);
            auto dependency_entity = _app->world.lookup(dependency_path.c_str());

            if (!dependency_entity.is_valid())
            {
                _app->get_logger()->warn("extensions::stages: dependency '{}' not found for stage '{}'", dependency_path, absolute_path);
                continue;
            }

            stage_entity.depends_on(dependency_entity);
            _app->get_logger()->debug("extensions::stages: stage '{}' depends_on '{}'", absolute_path, dependency_path);
        }
    }

    void stages::destroy(std::string_view name)
    {

        const std::string absolute_path = "::stages::" + std::string(name);
        _app->get_logger()->info("extensions::stages: destroying stage '{}'", absolute_path);

        auto stage_entity = _app->world.lookup(absolute_path.c_str());
        if (!stage_entity.is_valid())
        {
            _app->get_logger()->warn("extensions::stages: stage '{}' not found", absolute_path);
            return;
        }

        stage_entity.destruct();
    }

    void stages::enable(std::string_view name)
    {

        const std::string absolute_path = "::stages::" + std::string(name);
        auto stage_entity = _app->world.lookup(absolute_path.c_str());

        if (!stage_entity.is_valid())
        {
            _app->get_logger()->warn("extensions::stages: stage '{}' not found for enable", absolute_path);
            return;
        }

        stage_entity.enable();
        _app->get_logger()->debug("extensions::stages: enabled stage '{}'", absolute_path);
    }

    void stages::disable(std::string_view name)
    {

        const std::string absolute_path = "::stages::" + std::string(name);
        auto stage_entity = _app->world.lookup(absolute_path.c_str());

        if (!stage_entity.is_valid())
        {
            _app->get_logger()->warn("extensions::stages: stage '{}' not found for disable", absolute_path);
            return;
        }

        stage_entity.disable();
        _app->get_logger()->debug("extensions::stages: disabled stage '{}'", absolute_path);
    }

    bool stages::exists(std::string_view name) const
    {


        const std::string absolute_path = "::stages::" + std::string(name);
        return _app->world.lookup(absolute_path.c_str()).is_valid();
    }

    bool stages::enabled(std::string_view name) const
    {


        const std::string absolute_path = "::stages::" + std::string(name);
        auto stage_entity = _app->world.lookup(absolute_path.c_str());
        return stage_entity.is_valid() && stage_entity.enabled();
    }
}
