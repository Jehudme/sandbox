#pragma once

#include "sandbox/core/extension.h"
#include <string_view>

namespace sandbox::extensions
{
    class triggers : public sandbox::extension
    {
    public:
        void initialize(const sandbox::properties& props) override;
        void finalize() override;

        template<typename... components>
        void create(std::string_view name, auto&& configuration_lambda, auto&& logic_lambda);

        template<typename... components>
        void subscribe_trigger_events();

        void destroy(std::string_view name);
        void enable(std::string_view name);
        void disable(std::string_view name);

        bool exists(std::string_view name) const;
        bool enabled(std::string_view name) const;
    };
}

#include "triggers_ext.inl"
