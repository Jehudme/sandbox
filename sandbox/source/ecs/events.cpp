#include "sandbox/ecs/events.h"
#include "sandbox/data/caches.h"
#include "sandbox/core/engine.h"
#include "sandbox/core/properties.h"

#include <vector>

namespace sandbox::extensions
{
    void events::initialize(const sandbox::properties& properties)
    {
        // Register the transient event tag explicitly.
        _app->world.template component<transient_event_tag>();

        // Create the cleanup system under a reserved internal path that does not collide
        // with user-controlled names.  Using the ::__internal:: scope keeps it out of
        // both ::events:: (managed by events::destroy/enable/disable) and ::systems::
        // (managed by systems::destroy/enable/disable), so it cannot be accidentally
        // removed or disabled by user code.
        //
        // The system runs at flecs::PostUpdate so transient entities live for exactly
        // one frame: they are created and emitted during the frame, then cleaned up
        // before the next frame begins.
        //
        // Entities are collected into a vector before destruction to avoid modifying
        // the iteration table while the iterator is still active.
        _app->world.template system<transient_event_tag>("::__internal::events_transient_cleanup")
            .kind(flecs::PostUpdate)
            .run([](flecs::iter& iterator) {
                std::vector<flecs::entity> to_destroy;
                while (iterator.next())
                {
                    for (auto i : iterator)
                    {
                        to_destroy.push_back(iterator.entity(i));
                    }
                }
                for (auto& entity : to_destroy)
                {
                    entity.destruct();
                }
            });
    }

    void events::finalize()
    {
    }

    void events::destroy(std::string_view name)
    {
        // Operate only on the ::events:: namespace where observer subscriptions live.
        const std::string absolute_path = "::events::" + std::string(name);
        auto subscriber_entity = _app->world.lookup(absolute_path.c_str());

        if (subscriber_entity.is_valid())
        {
            subscriber_entity.destruct();
        }
    }

    void events::enable(std::string_view name)
    {
        const std::string absolute_path = "::events::" + std::string(name);

        // Cache-aside lookup
        flecs::entity subscriber_entity;
        auto* cache = _app->get_extension<extensions::caches>("caches");
        if (cache)
            subscriber_entity = cache->get(name);

        if (!subscriber_entity.is_valid())
        {
            subscriber_entity = _app->world.lookup(absolute_path.c_str());
            if (subscriber_entity.is_valid() && cache)
                cache->save(subscriber_entity);
        }

        if (subscriber_entity.is_valid())
            subscriber_entity.enable();
    }

    void events::disable(std::string_view name)
    {
        const std::string absolute_path = "::events::" + std::string(name);

        // Cache-aside lookup
        flecs::entity subscriber_entity;
        auto* cache = _app->get_extension<extensions::caches>("caches");
        if (cache)
            subscriber_entity = cache->get(name);

        if (!subscriber_entity.is_valid())
        {
            subscriber_entity = _app->world.lookup(absolute_path.c_str());
            if (subscriber_entity.is_valid() && cache)
                cache->save(subscriber_entity);
        }

        if (subscriber_entity.is_valid())
            subscriber_entity.disable();
    }

    bool events::exists(std::string_view name) const
    {
        // Check only the ::events:: namespace for observer subscriptions.
        auto* cache = _app->get_extension<extensions::caches>("caches");
        if (cache)
        {
            auto cached_entity = cache->get(name);
            if (cached_entity.is_valid())
                return true;
        }

        const std::string absolute_path = "::events::" + std::string(name);
        auto subscriber_entity = _app->world.lookup(absolute_path.c_str());
        if (subscriber_entity.is_valid() && cache)
            cache->save(subscriber_entity);
        return subscriber_entity.is_valid();
    }

    bool events::enabled(std::string_view name) const
    {
        // Cache-aside lookup
        flecs::entity subscriber_entity;
        auto* cache = _app->get_extension<extensions::caches>("caches");
        if (cache)
            subscriber_entity = cache->get(name);

        if (!subscriber_entity.is_valid())
        {
            const std::string absolute_path = "::events::" + std::string(name);
            subscriber_entity = _app->world.lookup(absolute_path.c_str());
            if (subscriber_entity.is_valid() && cache)
                cache->save(subscriber_entity);
        }

        return subscriber_entity.is_valid() && subscriber_entity.enabled();
    }
}
