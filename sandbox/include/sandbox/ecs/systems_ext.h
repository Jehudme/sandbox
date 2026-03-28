#pragma once

#include "sandbox/core/extension.h"
#include "sandbox/core/engine.h"
#include <string_view>

namespace sandbox::extensions
{
    class systems : public sandbox::extension
    {
    public:
        void initialize(const sandbox::properties& props) override;
        void finalize() override;

        template<typename... components>
        void create(std::string_view name, std::string_view stage, auto&& configuration_lambda, auto&& logic_lambda);

        template<typename... components>
        void create(std::string_view name, std::string_view stage, auto&& logic_lambda);

        template<typename... components>
        void subscribe_system_events();

        void destroy(std::string_view name);
        void enable(std::string_view name);
        void disable(std::string_view name);

        bool exists(std::string_view name) const;
        bool enabled(std::string_view name) const;
    };
}

#include "systems_ext.inl"
