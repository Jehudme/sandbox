#pragma once

#include "sandbox/core/extension.h"
#include <string_view>
#include <string>
#include <unordered_map>
#include <flecs.h>

namespace sandbox::extensions
{
    class caches : public sandbox::extension
    {
    public:
        struct cache_flag
        {
        };

        struct cache_data
        {
            flecs::entity entity;

            float lifetime;
            float last_access_time;
        };

        void initialize(const sandbox::properties& properties) override;
        void finalize() override;

        void save(flecs::entity entity, float lifetime = -1.0f);
        void free(std::string_view name);

        flecs::entity get(std::string_view name);

    private:
        std::unordered_map<std::string, cache_data> _caches;
    };
}