//
// Created by jehud on 2026-04-06.
//

#include "sandbox/core/engine.h"

#include "sandbox/core/plugin.h"
#include "sandbox/core/type_registry.h"

namespace sandbox
{
    void engine::initialize(const properties& props)
    {
        entity entt = m_world.entity("::internal");

        entt.set<properties>(props);
    }

    void engine::finalize()
    {
    }

    void engine::create_plugin(std::string_view name, std::string_view identifier)
    {
    }

    void engine::delete_plugin(std::string_view name)
    {
    }

    const properties& engine::get_properties() const
    {
        entity properties_entity = m_world.entity("::internal");

        if (properties_entity.has<properties>()) {
            return properties_entity.get<properties>();
        } else {
            properties_entity.set<properties>(properties{});
            return properties_entity.get<properties>();
        }
    }

} // sandbox