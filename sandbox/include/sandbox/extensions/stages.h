#pragma once

#include "sandbox/core/extension.h"
#include <vector>
#include <string_view>

namespace sandbox::extensions
{
    class stages : public sandbox::extension
    {
    public:
        using dependencies = std::vector<std::string_view>;

        void initialize(sandbox::engine& app, const sandbox::properties& props) override;
        void finalize(sandbox::engine& app) override;

        void create(std::string_view name, const dependencies& stage_dependencies = {});
        void destroy(std::string_view name);
        void enable(std::string_view name);
        void disable(std::string_view name);

        bool exists(std::string_view name) const;
        bool enabled(std::string_view name) const;
    };
}