#pragma once

#include "sandbox/core/extension.h"
#include <string_view>

namespace sandbox::extensions
{
    class systems : public sandbox::extension
    {
    public:
        void initialize(sandbox::engine& app, const sandbox::properties& props) override;
        void finalize(sandbox::engine& app) override;

        template<typename... components>
        void create(std::string_view name, std::string_view stage, auto&& configuration_lambda, auto&& logic_lambda);

        template<typename... components>
        void create(std::string_view name, std::string_view stage, auto&& logic_lambda);

        void destroy(std::string_view name);
        void enable(std::string_view name);
        void disable(std::string_view name);

        bool exists(std::string_view name) const;
        bool enabled(std::string_view name) const;
    };
}

#include "systems.inl"