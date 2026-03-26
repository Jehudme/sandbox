#pragma once

#include "sandbox/core/extension.h"

namespace sanbox::extensions
{
    class triggers : public sandbox::extension
    {
    public:
        using logic_lambda = std::function<void(flecs::iter&)>;
        using configuration_lambda = std::function<void(flecs::entity&)>; // not correct it supose to ne like an builder to chose at wich event, ect ect it get trigger

        template<typename... components>
        void create(std::string_view name, configuration_lambda&& configuration_callback, auto&& logic_callback);

        void destroy(std::string_view name);
        void disable(std::string_view name);

        void enable(std::string_view name);

        bool exists(std::string_view name) const;
        bool enabled(std::string_view name) const;

    };
}