#pragma once

#include <flecs.h>

#include "extension.h"
#include "sandbox/data/caches_ext.h"

namespace sandbox
{
    template<typename derived_type>
    derived_type* engine::get_extension(std::string_view category)
    {
        if (category != "caches")
        {
            if (auto* cache = get_extension<extensions::caches>("caches"))
            {
                auto cached_entity = cache->get(category);
                if (cached_entity.is_valid()
                    && cached_entity.template has<std::unique_ptr<sandbox::extension>>())
                {
                    const auto& ptr = cached_entity.template get<std::unique_ptr<sandbox::extension>>();
                    if (ptr && ptr->_app == this)
                        return static_cast<derived_type*>(ptr.get());
                }
            }
        }

        const std::string absolute_path = "::extensions::" + std::string(category);
        auto extension_entity = world.lookup(absolute_path.c_str());

        if (extension_entity.is_valid()
            && extension_entity.template has<std::unique_ptr<sandbox::extension>>())
        {
            const auto& extension_pointer = extension_entity.template get<std::unique_ptr<sandbox::extension>>();
            if (extension_pointer && extension_pointer->_app == this)
            {
                auto* raw = static_cast<derived_type*>(extension_pointer.get());
                if (category != "caches")
                {
                    if (auto* cache = get_extension<extensions::caches>("caches"))
                        cache->save(extension_entity);
                }
                return raw;
            }
        }
        return nullptr;
    }
}
