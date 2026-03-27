#pragma once

#include "sandbox/core/extension.h"
#include <string_view>

namespace sandbox::extensions
{
    class storage : public sandbox::extension
    {
    public:
        void initialize(sandbox::engine& app, const sandbox::properties& props) override;
        void finalize(sandbox::engine& app) override;

        template<typename base_type, typename... constructor_arguments>
        void create(std::string_view name, constructor_arguments&&... arguments);

        void destroy(std::string_view name);

        template<typename base_type>
        base_type* get(std::string_view name);

        bool exists(std::string_view name) const;
    };
}

#include "storage.inl"