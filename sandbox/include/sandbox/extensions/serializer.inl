#pragma once

#include <flecs.h>
#include <string_view>

namespace sandbox::extensions
{
    inline void request_save(flecs::entity entity, std::string_view filename)
    {
        entity.set<serializer::save_request>({std::string(filename)});
    }

    inline void request_load(flecs::entity entity, std::string_view filename)
    {
        entity.set<serializer::load_request>({std::string(filename)});
    }

    inline void mark_persistent(flecs::entity entity)
    {
        entity.add<serializer::persistent_tag>();
    }
}
