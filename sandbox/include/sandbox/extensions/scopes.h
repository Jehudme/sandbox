#pragma once

#include "sandbox/core/extension.h"
#include <vector>
#include <string_view>

namespace sandbox::extensions
{
    class scopes : public sandbox::extension
    {
    public:
        using scope_path = std::vector<std::string_view>;

        void initialize(const sandbox::properties& props) override;
        void finalize() override;

        void set_scope(const scope_path& desired_path);
        void push_scope(const scope_path& desired_path);
    };
}
