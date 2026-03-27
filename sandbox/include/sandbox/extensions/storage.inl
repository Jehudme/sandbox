#pragma once

#include "logger.h"
#include "sandbox/core/engine.h"

namespace sandbox::extensions
{
    template<typename base_type, typename... constructor_arguments>
    void storage::create(std::string_view name, constructor_arguments&&... arguments)
    {

        const std::string absolute_path = "::objects::" + std::string(name);
        _app->get_logger()->info("extensions::storage: creating object '{}'", absolute_path);

        auto object_entity = _app->world.entity(absolute_path.c_str());
        if (!object_entity.is_valid())
        {
            _app->get_logger()->error("extensions::storage: failed to create entity '{}'", absolute_path);
            return;
        }

        object_entity.template set<base_type>({std::forward<constructor_arguments>(arguments)...});

        _app->get_logger()->debug("extensions::storage: created object '{}'", absolute_path);
    }

    template<typename base_type>
    base_type* storage::get(std::string_view name)
    {
        if (!_app)
        {
            return nullptr;
        }

        const std::string absolute_path = "::objects::" + std::string(name);
        auto object_entity = _app->world.lookup(absolute_path.c_str());

        if (!object_entity.is_valid())
        {
            _app->get_logger()->warn("extensions::storage: object '{}' not found", absolute_path);
            return nullptr;
        }

        if (!object_entity.template has<base_type>())
        {
            _app->get_logger()->warn("extensions::storage: object '{}' does not have requested component", absolute_path);
            return nullptr;
        }

        return &object_entity.template get_mut<base_type>();
    }
}
