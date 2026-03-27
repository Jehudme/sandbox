#include "sandbox/data/caches_ext.h"
#include "sandbox/core/engine.h"
#include "sandbox/diagnostics/logger.h"
#include "sandbox/ecs/triggers_ext.h"
#include "sandbox/ecs/systems_ext.h"

#include <vector>

namespace sandbox::extensions
{
    void caches::initialize(const sandbox::properties& properties)
    {
        _app->world.component<cache_flag>();

        // Create an OnRemove observer so that whenever cache_flag is removed from an entity
        // (including during entity destruction), the cache entry is automatically cleaned up.
        if (auto* ext_triggers = _app->get_extension<extensions::triggers>("triggers"))
        {
            ext_triggers->create<cache_flag>("cache_flag_removed",
                [](auto& builder) { builder.event(flecs::OnRemove); },
                [this](flecs::iter& it) {
                    while (it.next())
                    {
                        for (auto i : it)
                        {
                            const char* name = it.entity(i).name();
                            this->free(name);
                        }
                    }
                }
            );
        }

        // Create the expiration system: every 5 seconds, evict any entry whose
        // (current_time - last_access_time) exceeds its lifetime.
        // A negative lifetime means the entry never expires.
        _app->get_extension<extensions::systems>("systems")->create<>(
            "cache_expiration_manager", "",
            [](auto& builder) { builder.kind(flecs::PostUpdate).interval(5.0f); },
            [this](flecs::iter& it) {
                const float current_time =
                    static_cast<float>(ecs_get_world_info(it.world())->world_time_total);

                std::vector<std::string> expired_names;
                for (const auto& [name, data] : _caches)
                {
                    if (data.lifetime >= 0.0f &&
                        (current_time - data.last_access_time) > data.lifetime)
                    {
                        expired_names.push_back(name);
                    }
                }

                for (const auto& name : expired_names)
                {
                    auto cache_it = _caches.find(name);
                    if (cache_it == _caches.end())
                        continue;

                    auto entity = cache_it->second.entity;
                    if (entity.is_valid())
                    {
                        entity.destruct(); // fires OnRemove<cache_flag> → free()
                    }
                    else
                    {
                        _caches.erase(cache_it);
                    }
                }
            }
        );
    }

    void caches::finalize()
    {
        // Destroy the expiration system and the OnRemove trigger before clearing the map
        // so that no callbacks fire with a dangling 'this' pointer after this object is gone.
        auto system_entity = _app->world.lookup("::systems::cache_expiration_manager");
        if (system_entity.is_valid())
            system_entity.destruct();

        auto trigger_entity = _app->world.lookup("::triggers::cache_flag_removed");
        if (trigger_entity.is_valid())
            trigger_entity.destruct();

        _caches.clear();
    }

    void caches::save(flecs::entity entity, float lifetime)
    {
        if (!entity.is_valid())
            return;

        const char* entity_name = entity.name();
        if (!entity_name || entity_name[0] == '\0')
        {
            if (auto* log = _app->get_logger())
                log->warn("extensions::caches: cannot cache entity without a name");
            return;
        }

        entity.add<cache_flag>();

        _caches[std::string(entity_name)] = {
            entity,
            lifetime,
            _app ? static_cast<float>(ecs_get_world_info(_app->world)->world_time_total) : 0.0f
        };
    }

    void caches::free(std::string_view name)
    {
        // Simply erase the map entry; the OnRemove trigger that calls this function
        // already handles the component-side cleanup.
        _caches.erase(std::string(name));
    }

    flecs::entity caches::get(std::string_view name)
    {
        auto it = _caches.find(std::string(name));
        if (it != _caches.end())
        {
            it->second.last_access_time =
                _app ? static_cast<float>(ecs_get_world_info(_app->world)->world_time_total) : 0.0f;
            return it->second.entity;
        }
        return flecs::entity();
    }
}