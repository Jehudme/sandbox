#include "sandbox/extensions/events.h"
#include "sandbox/core/engine.h"
#include "sandbox/filesystem/properties.h"
#include "sandbox/extensions/logger.h"
#include "sandbox/extensions/systems.h"

namespace sandbox::extensions
{
    void events::initialize(const sandbox::properties& properties)
    {
        auto* systems_extension = _app->get_systems();
        if (!systems_extension)
        {
            return;
        }

        // Register the tag explicitly
        _app->world.template component<transient_event_tag>();

        systems_extension->create<transient_event_tag>(
            "event_automatic_cleanup",
            "PostUpdate",
            [](flecs::iter& iterator) {
                while (iterator.next())
                {
                    for (auto i : iterator)
                    {
                        iterator.entity(i).destruct();
                    }
                }
            }
        );
    }

    void events::finalize()
    {
    }

    void events::destroy(std::string_view name)
    {
        const std::string absolute_path = "::events::" + std::string(name);
        auto subscriber_entity = _app->world.lookup(absolute_path.c_str());

        if (subscriber_entity.is_valid())
        {
            subscriber_entity.destruct();
        }

        auto* systems_extension = _app->get_systems();
        if (systems_extension && systems_extension->exists(name))
        {
            systems_extension->destroy(name);
        }
    }

    void events::enable(std::string_view name)
    {
        const std::string absolute_path = "::events::" + std::string(name);
        auto subscriber_entity = _app->world.lookup(absolute_path.c_str());

        if (subscriber_entity.is_valid())
        {
            subscriber_entity.enable();
        }

        auto* systems_extension = _app->get_systems();
        if (systems_extension && systems_extension->exists(name))
        {
            systems_extension->enable(name);
        }
    }

    void events::disable(std::string_view name)
    {
        const std::string absolute_path = "::events::" + std::string(name);
        auto subscriber_entity = _app->world.lookup(absolute_path.c_str());

        if (subscriber_entity.is_valid())
        {
            subscriber_entity.disable();
        }

        auto* systems_extension = _app->get_systems();
        if (systems_extension && systems_extension->exists(name))
        {
            systems_extension->disable(name);
        }
    }

    bool events::exists(std::string_view name) const
    {
        const std::string absolute_path = "::events::" + std::string(name);
        bool immediate_exists = _app->world.lookup(absolute_path.c_str()).is_valid();

        auto* systems_extension = _app->get_systems();
        bool staged_exists = systems_extension && systems_extension->exists(name);

        return immediate_exists || staged_exists;
    }

    bool events::enabled(std::string_view name) const
    {
        const std::string absolute_path = "::events::" + std::string(name);
        auto subscriber_entity = _app->world.lookup(absolute_path.c_str());

        bool is_enabled = false;
        if (subscriber_entity.is_valid() && subscriber_entity.enabled())
        {
            is_enabled = true;
        }

        auto* systems_extension = _app->get_systems();
        if (systems_extension && systems_extension->exists(name) && systems_extension->enabled(name))
        {
            is_enabled = true;
        }

        return is_enabled;
    }
}