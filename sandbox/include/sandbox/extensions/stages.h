#pragma once

#include "sandbox/core/extension.h"

namespace sanbox::extensions
{
    class stages : public sandbox::extension
    {
    public:
        using dependencies = std::vector<std::string_view>;

        void create(std::string_view name, const dependencies& stage_dependencies = {});
        void destroy(std::string_view name);
        void disable(std::string_view name);
        void enable(std::string_view name);

        bool exists(std::string_view name) const;
        bool enabled(std::string_view name) const;

    };
}