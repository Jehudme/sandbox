#include "sandbox/ecs/triggers.h"
#include "sandbox/data/caches.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/properties.h"

namespace sandbox::extensions
{
    void triggers::initialize(const sandbox::properties& props)
    {
    }

    void triggers::finalize()
    {
    }

    void triggers::destroy(std::string_view name)
    {
        if (!_app) return;

        const std::string absolute_path = "::triggers::" + std::string(name);

        if (auto* log = _app->get_logger())
            log->info("extensions::triggers: destroying trigger '{}'", absolute_path);

        auto trigger_entity = _app->world.lookup(absolute_path.c_str());
        if (!trigger_entity.is_valid())
        {
            if (auto* log = _app->get_logger())
                log->warn("extensions::triggers: trigger '{}' not found", absolute_path);
            return;
        }

        trigger_entity.destruct();
    }

    void triggers::enable(std::string_view name)
    {
        if (!_app) return;

        const std::string absolute_path = "::triggers::" + std::string(name);

        // Cache-aside lookup
        flecs::entity trigger_entity;
        auto* cache = _app->get_extension<extensions::caches>("caches");
        if (cache)
            trigger_entity = cache->get(name);

        if (!trigger_entity.is_valid())
        {
            trigger_entity = _app->world.lookup(absolute_path.c_str());
            if (trigger_entity.is_valid() && cache)
                cache->save(trigger_entity);
        }

        if (!trigger_entity.is_valid())
        {
            if (auto* log = _app->get_logger())
                log->warn("extensions::triggers: trigger '{}' not found for enable", absolute_path);
            return;
        }

        trigger_entity.enable();

        if (auto* log = _app->get_logger())
            log->debug("extensions::triggers: enabled trigger '{}'", absolute_path);
    }

    void triggers::disable(std::string_view name)
    {
        if (!_app) return;

        const std::string absolute_path = "::triggers::" + std::string(name);

        // Cache-aside lookup
        flecs::entity trigger_entity;
        auto* cache = _app->get_extension<extensions::caches>("caches");
        if (cache)
            trigger_entity = cache->get(name);

        if (!trigger_entity.is_valid())
        {
            trigger_entity = _app->world.lookup(absolute_path.c_str());
            if (trigger_entity.is_valid() && cache)
                cache->save(trigger_entity);
        }

        if (!trigger_entity.is_valid())
        {
            if (auto* log = _app->get_logger())
                log->warn("extensions::triggers: trigger '{}' not found for disable", absolute_path);
            return;
        }

        trigger_entity.disable();

        if (auto* log = _app->get_logger())
            log->debug("extensions::triggers: disabled trigger '{}'", absolute_path);
    }

    bool triggers::exists(std::string_view name) const
    {
        if (!_app) return false;

        // Cache-aside lookup
        auto* cache = _app->get_extension<extensions::caches>("caches");
        if (cache)
        {
            auto cached_entity = cache->get(name);
            if (cached_entity.is_valid())
                return true;
        }

        const std::string absolute_path = "::triggers::" + std::string(name);
        auto trigger_entity = _app->world.lookup(absolute_path.c_str());
        if (trigger_entity.is_valid() && cache)
            cache->save(trigger_entity);
        return trigger_entity.is_valid();
    }

    bool triggers::enabled(std::string_view name) const
    {
        if (!_app) return false;

        // Cache-aside lookup
        flecs::entity trigger_entity;
        auto* cache = _app->get_extension<extensions::caches>("caches");
        if (cache)
            trigger_entity = cache->get(name);

        if (!trigger_entity.is_valid())
        {
            const std::string absolute_path = "::triggers::" + std::string(name);
            trigger_entity = _app->world.lookup(absolute_path.c_str());
            if (trigger_entity.is_valid() && cache)
                cache->save(trigger_entity);
        }

        return trigger_entity.is_valid() && trigger_entity.enabled();
    }
}
