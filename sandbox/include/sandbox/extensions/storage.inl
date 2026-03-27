#pragma once

#include "logger.h"
#include "sandbox/core/engine.h"

namespace sandbox::extensions
{
    template<typename base_type, typename... constructor_arguments>
    void storage::create(std::string_view name, constructor_arguments&&... arguments)
    {
        if (!_app) return;

        const std::string absolute_path = "::objects::" + std::string(name);

        if (auto* log = _app->get_logger())
            log->info("extensions::storage: creating object '{}'", absolute_path);

        auto object_entity = _app->world.entity(absolute_path.c_str());
        if (!object_entity.is_valid())
        {
            if (auto* log = _app->get_logger())
                log->error("extensions::storage: failed to create entity '{}'", absolute_path);
            return;
        }

        // Construct explicitly to support non-aggregate and non-brace-constructible types.
        object_entity.template set<base_type>(base_type(std::forward<constructor_arguments>(arguments)...));

        if (auto* log = _app->get_logger())
            log->debug("extensions::storage: created object '{}'", absolute_path);
    }

    template<typename base_type>
    base_type* storage::get(std::string_view name)
    {
        if (!_app) return nullptr;

        const std::string absolute_path = "::objects::" + std::string(name);
        auto object_entity = _app->world.lookup(absolute_path.c_str());

        if (!object_entity.is_valid())
        {
            if (auto* log = _app->get_logger())
                log->warn("extensions::storage: object '{}' not found", absolute_path);
            return nullptr;
        }

        if (!object_entity.template has<base_type>())
        {
            if (auto* log = _app->get_logger())
                log->warn("extensions::storage: object '{}' does not have requested component", absolute_path);
            return nullptr;
        }

        // NOTE: The returned pointer is valid only as long as the entity exists and its
        // component is not removed or replaced.  Do not cache this pointer across frames
        // or across structural ECS changes (add/remove components, entity destruction).
        return &object_entity.template get_mut<base_type>();
    }
}

