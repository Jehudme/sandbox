#pragma once
#include "flecs/addons/cpp/world.hpp"
#include "sandbox/core/extension.h"

namespace sandbox::extensions
{
    class systems : public sandbox::extension
    {
    public:
        template<typename... components>
        void create(std::string_view name, std::string_view stage, auto&& configuration_lambda, auto&& logic_lambda);

        template<typename... components>
        void create(std::string_view name, std::string_view stage, auto&& logic_lambda); // lazy implementation

        void destroy(std::string_view name);
        void disable(std::string_view name);
        void enable(std::string_view name);

        bool exists(std::string_view name) const;
        bool enabled(std::string_view name) const;
    };
}