#pragma once

#include <flecs.h>

#include "extension.h"

namespace sandbox
{
    template<typename derived_type>
    derived_type* engine::get_extension(std::string_view category)
    {
        std::string absolute_path = "::extensions::" + std::string(category);
        auto extension_entity = world.lookup(absolute_path.c_str());

        if (extension_entity.is_valid()
            && extension_entity.template has<std::unique_ptr<sandbox::extension>>())
        {
            const auto& extension_pointer = extension_entity.template get<std::unique_ptr<sandbox::extension>>();
            if (extension_pointer && extension_pointer->_app == this)
            {
                return static_cast<derived_type*>(extension_pointer.get());
            }
        }
        return nullptr;
    }
}
