#include "sandbox/ecs/systems_ext.h"
#include "sandbox/ecs/systems_evt.h"
#include "sandbox/ecs/events_ext.h"
#include "sandbox/data/caches_ext.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/properties.h"

namespace sandbox::extensions
{
    void systems::initialize(const sandbox::properties& props)
    {
        if (auto* ext_events = _app->get_extension<extensions::events>("events"))
        {
            subscribe_system_events<>();

            ext_events->subscribe<sandbox::ecs::destroy_system_evt>("systems_destroy", [this](sandbox::ecs::destroy_system_evt& e) {
                this->destroy(e.name);
            });

            ext_events->subscribe<sandbox::ecs::enable_system_evt>("systems_enable", [this](sandbox::ecs::enable_system_evt& e) {
                this->enable(e.name);
            });

            ext_events->subscribe<sandbox::ecs::disable_system_evt>("systems_disable", [this](sandbox::ecs::disable_system_evt& e) {
                this->disable(e.name);
            });

            ext_events->subscribe<sandbox::ecs::system_exists_evt>("systems_exists", [this](sandbox::ecs::system_exists_evt& e) {
                if (e.out_exists)
                    *e.out_exists = this->exists(e.name);
            });

            ext_events->subscribe<sandbox::ecs::system_enabled_evt>("systems_enabled", [this](sandbox::ecs::system_enabled_evt& e) {
                if (e.out_enabled)
                    *e.out_enabled = this->enabled(e.name);
            });
        }
    }

    void systems::finalize()
    {
    }

    void systems::destroy(std::string_view name)
    {
        if (!_app) return;

        const std::string absolute_path = "::systems::" + std::string(name);

        if (auto* log = _app->get_logger())
            log->info("extensions::systems: destroying system '{}'", absolute_path);

        auto system_entity = _app->world.lookup(absolute_path.c_str());
        if (!system_entity.is_valid())
        {
            if (auto* log = _app->get_logger())
                log->warn("extensions::systems: system '{}' not found", absolute_path);
            return;
        }

        system_entity.destruct();
    }

    void systems::enable(std::string_view name)
    {
        if (!_app) return;

        const std::string absolute_path = "::systems::" + std::string(name);

        flecs::entity system_entity;
        auto* cache = _app->get_extension<extensions::caches>("caches");
        if (cache)
            system_entity = cache->get(name);

        if (!system_entity.is_valid())
        {
            system_entity = _app->world.lookup(absolute_path.c_str());
            if (system_entity.is_valid() && cache)
                cache->save(system_entity);
        }

        if (!system_entity.is_valid())
        {
            if (auto* log = _app->get_logger())
                log->warn("extensions::systems: system '{}' not found for enable", absolute_path);
            return;
        }

        system_entity.enable();

        if (auto* log = _app->get_logger())
            log->debug("extensions::systems: enabled system '{}'", absolute_path);
    }

    void systems::disable(std::string_view name)
    {
        if (!_app) return;

        const std::string absolute_path = "::systems::" + std::string(name);

        flecs::entity system_entity;
        auto* cache = _app->get_extension<extensions::caches>("caches");
        if (cache)
            system_entity = cache->get(name);

        if (!system_entity.is_valid())
        {
            system_entity = _app->world.lookup(absolute_path.c_str());
            if (system_entity.is_valid() && cache)
                cache->save(system_entity);
        }

        if (!system_entity.is_valid())
        {
            if (auto* log = _app->get_logger())
                log->warn("extensions::systems: system '{}' not found for disable", absolute_path);
            return;
        }

        system_entity.disable();

        if (auto* log = _app->get_logger())
            log->debug("extensions::systems: disabled system '{}'", absolute_path);
    }

    bool systems::exists(std::string_view name) const
    {
        if (!_app) return false;

        auto* cache = _app->get_extension<extensions::caches>("caches");
        if (cache)
        {
            auto cached_entity = cache->get(name);
            if (cached_entity.is_valid())
                return true;
        }

        const std::string absolute_path = "::systems::" + std::string(name);
        auto system_entity = _app->world.lookup(absolute_path.c_str());
        if (system_entity.is_valid() && cache)
            cache->save(system_entity);
        return system_entity.is_valid();
    }

    bool systems::enabled(std::string_view name) const
    {
        if (!_app) return false;

        flecs::entity system_entity;
        auto* cache = _app->get_extension<extensions::caches>("caches");
        if (cache)
            system_entity = cache->get(name);

        if (!system_entity.is_valid())
        {
            const std::string absolute_path = "::systems::" + std::string(name);
            system_entity = _app->world.lookup(absolute_path.c_str());
            if (system_entity.is_valid() && cache)
                cache->save(system_entity);
        }

        return system_entity.is_valid() && system_entity.enabled();
    }
}
