#pragma once

#include "sandbox/core/extension.h"
#include <vector>
#include <string>
#include <flecs.h>

namespace sandbox::extensions
{
    class dependencies : public sandbox::extension
    {
    public:
        struct state {
            std::vector<std::string> requirements;
        };

        struct is_ready_tag {};

        void initialize(const sandbox::properties& properties) override;
        void finalize() override;

        void require(std::string_view extension_name);
        void validate();
    };
}
