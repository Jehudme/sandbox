#pragma once

namespace sandbox::extensions
{
    template<typename Type>
    void caches::save(std::string_view name, flecs::entity entity, float lifetime)
    {
        if (!entity.is_valid() || !entity.has<Type>()) return;

        // 1. Tag the COMPONENT entity as 'cached'
        // This allows our generic trigger to find it during OnRemove
        std::string absolute_path = "::objects::" + std::string(name);
        _app->world.component<Type>().template add<cache_flag>();

        // 2. Store the raw pointer and metadata
        _caches_map[std::string(name)] = {
            const_cast<Type*>(entity.get<Type>()),
            lifetime,
            entity
        };
    }

    template<typename Type>
    Type* caches::get_ptr(std::string_view name)
    {
        auto it = _caches_map.find(std::string(name));
        if (it != _caches_map.end())
        {
            return static_cast<Type*>(it->second.ptr);
        }
        return nullptr;
    }
}