#pragma once

#include <string>
#include <string_view>
#include <flecs.h>

namespace sandbox::data
{
    /**
     * @brief Events and Commands for the caches extension.
     */

    struct save_cache_evt
    {
        flecs::entity entity;
        float lifetime;

        static save_cache_evt create(flecs::entity entity, float lifetime = -1.0f)
        {
            return {entity, lifetime};
        }
    };

    struct free_cache_evt
    {
        std::string name;

        static free_cache_evt create(std::string_view name)
        {
            return {std::string(name)};
        }
    };

    struct get_cache_evt
    {
        std::string name;
        flecs::entity* out_entity;

        static get_cache_evt create(std::string_view name, flecs::entity* out_entity)
        {
            return {std::string(name), out_entity};
        }
    };
}

