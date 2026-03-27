#pragma once

#include "sandbox/core/extension.h"
#include <unordered_map>
#include <string>
#include <flecs.h>

namespace sandbox::extensions
{
    class diagnostics : public sandbox::extension
    {
    public:
        struct metric {
            float last_ms;
            float avg_ms;
        };

        struct state {
            std::unordered_map<std::string, metric> system_times;
            float fps;
        };

        // Component: Attach to a system entity to track its frame time automatically
        struct profile_tag {};

        void initialize(const sandbox::properties& properties) override;
        void finalize() override;
    };
}